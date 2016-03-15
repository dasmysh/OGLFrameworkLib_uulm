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
        FrameBufferDescriptor(std::vector<FrameBufferTextureDescriptor>({ FrameBufferTextureDescriptor{ texDesc, GL_TEXTURE_CUBE_MAP } }),
        std::vector<RenderBufferDescriptor>{ { GL_DEPTH_COMPONENT32F } })),
    perspective_(glm::perspective(glm::half_pi<float>(), 1.0f, nearZ, farZ)),
    perspectiveUBO_(std::make_unique<GLUniformBuffer>(perspectiveProjectionUBBName, static_cast<unsigned int>(sizeof(glm::mat4)), app->GetUBOBindingPoints())),
    sphProgram(app->GetGPUProgramManager()->GetResource("shader/envmap/cubetospherical.cp")),
    sphUniformIds(sphProgram->GetUniformLocations({ "cubeMap", "sphericalTex" })),
    irrProgram(app->GetGPUProgramManager()->GetResource("shader/envmap/sphericalirradiance.cp,NUM_MATS 2,BRDF_TYPE torranceSparrow")),
    irrUniformIds(irrProgram->GetUniformLocations({ "sphericalTex", "irradianceMap", "sourceRectMin", "sourceRectMax", "materialIdx" }))
    {
        irrProgram->BindUniformBlock("materialsBuffer", *app->GetUBOBindingPoints());
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

    void EnvironmentMapGenerator::DrawToCubeMap(const glm::vec3& position, std::function<void(GLBatchRenderTarget&)> batch)
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
    }

    std::unique_ptr<GLTexture> EnvironmentMapGenerator::GenerateIrradianceMap(const glm::uvec2& sphericalRes, int irrMaterialId)
    {
        const unsigned int irrMipLevel = 3;

        auto oldSeamless = glIsEnabled(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        auto sphEnvMapDesc = cubeMapRT_.GetTextures()[0]->GetDescriptor();
        TextureRAII texID;
        OGL_CALL(glBindTexture, GL_TEXTURE_2D, texID);
        OGL_CALL(glTexImage2D, GL_TEXTURE_2D, 0, sphEnvMapDesc.internalFormat, sphericalRes.x, sphericalRes.y, 0, sphEnvMapDesc.format, sphEnvMapDesc.type, nullptr);
        auto sphEnvMap = std::make_unique<GLTexture>(std::move(texID), GL_TEXTURE_2D, sphEnvMapDesc);

        sphProgram->UseProgram();
        sphProgram->SetUniform(sphUniformIds[0], 0);
        sphProgram->SetUniform(sphUniformIds[1], 0);
        cubeMapRT_.GetTextures()[0]->ActivateTexture(GL_TEXTURE0);
        sphEnvMap->ActivateImage(0, 0, GL_WRITE_ONLY);
        OGL_CALL(glDispatchCompute, sphericalRes.x / 32, sphericalRes.y / 16, 1);
        OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
        OGL_SCALL(glFinish);

        sphEnvMap->GenerateMipMaps();
        auto dim = sphEnvMap->GetLevelDimensions(irrMipLevel);

        auto irrMap = std::make_unique<GLTexture>(dim.x, dim.y, sphEnvMapDesc, nullptr);

        irrProgram->UseProgram();
        irrProgram->SetUniform(irrUniformIds[0], 0);
        irrProgram->SetUniform(irrUniformIds[1], 1);
        irrProgram->SetUniform(irrUniformIds[4], irrMaterialId);
        sphEnvMap->ActivateImage(0, irrMipLevel, GL_READ_ONLY);
        irrMap->ActivateImage(1, 0, GL_READ_WRITE);

        const unsigned int chunkSize = 8;
        for (unsigned int ix = 0; ix < dim.x; ix += chunkSize) {
            for (unsigned int iy = 0; iy < dim.y; iy += chunkSize) {
                irrProgram->SetUniform(irrUniformIds[2], glm::ivec2(ix, iy));
                irrProgram->SetUniform(irrUniformIds[3], glm::ivec2(ix + chunkSize, iy + chunkSize));

                OGL_CALL(glDispatchCompute, dim.x / 32, dim.y / 16, 1);
                OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
                OGL_SCALL(glFinish);
            }
        }

        if (!oldSeamless) glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        return std::move(irrMap);
        // return std::move(sphEnvMap);
    }
}
