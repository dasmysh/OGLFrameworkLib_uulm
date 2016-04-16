/**
 * @file   EnvironmentMapGenerator.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.03.02
 *
 * @brief  Implementation of the environment map generator.
 */

#include "EnvironmentMapGenerator.h"
#include <glm/gtc/matrix_transform.hpp>
#include "gfx/glrenderer/GLUniformBuffer.h"
#include "app/ApplicationBase.h"

namespace cgu {

    EnvironmentMapGenerator::EnvironmentMapGenerator(unsigned int size, float nearZ, float farZ,
        const TextureDescriptor& texDesc, ApplicationBase* app) :
    cubeMapRT_(size, size,
        FrameBufferDescriptor(std::vector<FrameBufferTextureDescriptor>({ FrameBufferTextureDescriptor{ texDesc, gl::GL_TEXTURE_CUBE_MAP } }),
        std::vector<RenderBufferDescriptor>{ { gl::GL_DEPTH_COMPONENT32F } })),
    perspective_(glm::perspective(glm::half_pi<float>(), 1.0f, nearZ, farZ)),
    perspectiveUBO_(std::make_unique<GLUniformBuffer>(perspectiveProjectionUBBName, static_cast<unsigned int>(sizeof(glm::mat4)), app->GetUBOBindingPoints())),
    sphProgram_(app->GetGPUProgramManager()->GetResource("shader/envmap/cubetospherical.cp")),
    sphUniformIds_(sphProgram_->GetUniformLocations({ "cubeMap", "sphericalTex" }))
    {
        auto sphEnvMapDesc = cubeMapRT_.GetTextures()[0]->GetDescriptor();
        TextureRAII texID;
        OGL_CALL(gl::glBindTexture, gl::GL_TEXTURE_2D, texID);
        OGL_CALL(gl::glTexImage2D, gl::GL_TEXTURE_2D, 0, static_cast<gl::GLint>(sphEnvMapDesc.internalFormat), 2 * size, size, 0, sphEnvMapDesc.format, sphEnvMapDesc.type, nullptr);
        sphEnvMap_ = std::make_unique<GLTexture>(std::move(texID), gl::GL_TEXTURE_2D, sphEnvMapDesc);
        sphEnvMap_->GenerateMipMaps();

        dir_.emplace_back(-1.0f, 0.0f, 0.0f);
        up_.emplace_back(0.0f, 1.0f, 0.0f);
        right_.emplace_back(0.0f, 0.0f, 1.0f);
        dir_.emplace_back(1.0f, 0.0f, 0.0f);
        up_.emplace_back(0.0f, 1.0f, 0.0f);
        right_.emplace_back(0.0f, 0.0f, -1.0f);

        dir_.emplace_back(0.0f, 1.0f, 0.0f);
        up_.emplace_back(0.0f, 0.0f, -1.0f);
        right_.emplace_back(1.0f, 0.0f, 0.0f);
        dir_.emplace_back(0.0f, -1.0f, 0.0f);
        up_.emplace_back(0.0f, 0.0f, 1.0f);
        right_.emplace_back(1.0f, 0.0f, 0.0f);
        
        dir_.emplace_back(0.0f, 0.0f, 1.0f);
        up_.emplace_back(0.0f, 1.0f, 0.0f);
        right_.emplace_back(1.0f, 0.0f, 0.0f);
        dir_.emplace_back(0.0f, 0.0f, -1.0f);
        up_.emplace_back(0.0f, 1.0f, 0.0f);
        right_.emplace_back(-1.0f, 0.0f, 0.0f);
    }

    EnvironmentMapGenerator::EnvironmentMapGenerator(const EnvironmentMapGenerator& rhs) :
        cubeMapRT_(rhs.cubeMapRT_),
        perspective_(rhs.perspective_),
        dir_(rhs.dir_),
        up_(rhs.up_),
        right_(rhs.right_),
        perspectiveUBO_(new GLUniformBuffer(*rhs.perspectiveUBO_))
    {
    }

    EnvironmentMapGenerator& EnvironmentMapGenerator::operator=(const EnvironmentMapGenerator& rhs)
    {
        if (this != &rhs) {
            EnvironmentMapGenerator tmp{ rhs };
            std::swap(*this, tmp);
        }
        return *this;
    }

    EnvironmentMapGenerator::EnvironmentMapGenerator(EnvironmentMapGenerator&& rhs) :
        cubeMapRT_(std::move(rhs.cubeMapRT_)),
        perspective_(std::move(rhs.perspective_)),
        dir_(std::move(rhs.dir_)),
        up_(std::move(rhs.up_)),
        right_(std::move(rhs.right_)),
        perspectiveUBO_(std::move(rhs.perspectiveUBO_))
    {
    }

    EnvironmentMapGenerator& EnvironmentMapGenerator::operator=(EnvironmentMapGenerator&& rhs)
    {
        if (&rhs != this) {
            this->~EnvironmentMapGenerator();
            cubeMapRT_ = std::move(rhs.cubeMapRT_);
            perspective_ = std::move(rhs.perspective_);
            dir_ = std::move(rhs.dir_);
            up_ = std::move(rhs.up_);
            right_ = std::move(rhs.right_);
            perspectiveUBO_ = std::move(rhs.perspectiveUBO_);
        }
        return *this;
    }

    EnvironmentMapGenerator::~EnvironmentMapGenerator() = default;

    void EnvironmentMapGenerator::Resize(unsigned int size)
    {
        cubeMapRT_.Resize(size, size);
    }

    void EnvironmentMapGenerator::DrawEnvMap(const glm::vec3& position, std::function<void(GLBatchRenderTarget&)> batch)
    {
        std::vector<unsigned int> drawBufferIndices(1, 0);
        auto perspectiveUBO = perspectiveUBO_.get();
        float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        for (auto i = 0; i < 6; ++i) {
            drawBufferIndices[0] = i;
            glm::mat4 view;
            view[0] = glm::vec4(right_[i], 0.0f);
            view[1] = glm::vec4(up_[i], 0.0f);
            view[2] = glm::vec4(dir_[i], 0.0f);
            view[3] = glm::vec4(-glm::dot(position, right_[i]), -glm::dot(position, up_[i]), -glm::dot(position, dir_[i]), 1.0f);
            auto viewProjection = perspective_ * glm::transpose(view);
            cubeMapRT_.BatchDraw(drawBufferIndices, [&clearColor, &batch, &perspectiveUBO, &viewProjection](cgu::GLBatchRenderTarget & brt){
                brt.Clear(static_cast<unsigned int>(cgu::ClearFlags::CF_RenderTarget) | static_cast<unsigned int>(cgu::ClearFlags::CF_Depth), clearColor, 1.0, 0);

                if (perspectiveUBO) {
                    perspectiveUBO->UploadData(0, sizeof(glm::mat4), &viewProjection);
                    perspectiveUBO->BindBuffer();
                }

                batch(brt);
            });
        }

        auto oldSeamless = gl::glIsEnabled(gl::GL_TEXTURE_CUBE_MAP_SEAMLESS);
        gl::glEnable(gl::GL_TEXTURE_CUBE_MAP_SEAMLESS);
        auto sphericalRes = sphEnvMap_->GetDimensions();

        sphProgram_->UseProgram();
        sphProgram_->SetUniform(sphUniformIds_[0], 0);
        sphProgram_->SetUniform(sphUniformIds_[1], 0);
        cubeMapRT_.GetTextures()[0]->ActivateTexture(gl::GL_TEXTURE0);
        sphEnvMap_->ActivateImage(0, 0, gl::GL_WRITE_ONLY);
        OGL_CALL(gl::glDispatchCompute, sphericalRes.x / 32, sphericalRes.y / 16, 1);
        OGL_CALL(gl::glMemoryBarrier, gl::GL_ALL_BARRIER_BITS);
        OGL_SCALL(gl::glFinish);

        sphEnvMap_->GenerateMipMaps();

        if (oldSeamless == gl::GL_FALSE) gl::glDisable(gl::GL_TEXTURE_CUBE_MAP_SEAMLESS);
    }

    std::unique_ptr<GLTexture> EnvironmentMapGenerator::GenerateIrradianceMap(unsigned int irrMipLevel) const
    {
        auto dim = sphEnvMap_->GetLevelDimensions(irrMipLevel);
        auto sphEnvMapDesc = cubeMapRT_.GetTextures()[0]->GetDescriptor();
        auto irrMap = std::make_unique<GLTexture>(dim.x, dim.y, sphEnvMapDesc, nullptr);


        return std::move(irrMap);
    }

    void EnvironmentMapGenerator::UpdateIrradianceMap(const GPUProgram* irrProgram, const std::vector<BindingLocation>& irrUniformIds, const GLTexture* irrMap, unsigned int irrMipLevel) const
    {
        auto dim = sphEnvMap_->GetLevelDimensions(irrMipLevel);
        irrProgram->SetUniform(irrUniformIds[0], 0);
        irrProgram->SetUniform(irrUniformIds[1], 1);
        sphEnvMap_->ActivateImage(0, irrMipLevel, gl::GL_READ_ONLY);
        irrMap->ActivateImage(1, 0, gl::GL_READ_WRITE);

        const unsigned int chunkSize = 8;
        for (unsigned int ix = 0; ix < dim.x; ix += chunkSize) {
            for (unsigned int iy = 0; iy < dim.y; iy += chunkSize) {
                irrProgram->SetUniform(irrUniformIds[2], glm::ivec2(ix, iy));
                irrProgram->SetUniform(irrUniformIds[3], glm::ivec2(ix + chunkSize, iy + chunkSize));

                OGL_CALL(gl::glDispatchCompute, dim.x / 32, dim.y / 16, 1);
                OGL_CALL(gl::glMemoryBarrier, gl::GL_ALL_BARRIER_BITS);
                OGL_SCALL(gl::glFinish);
            }
        }
    }
}
