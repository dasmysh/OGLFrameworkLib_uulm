/**
 * @file   ShadowMap.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.01.08
 *
 * @brief  Implementation of the shadow map class.
 */

#include "ShadowMap.h"
#include "app/ApplicationBase.h"
#include "gfx/glrenderer/GLRenderTarget.h"
#include "SpotLight.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "gfx/glrenderer/GLTexture.h"

namespace cgu {

    ShadowMap::ShadowMap(const glm::uvec2& size, unsigned int components, const SpotLight& light, const std::shared_ptr<GPUProgram>& smProgram,
        const std::shared_ptr<GPUProgram>& filterProgram, ApplicationBase* app) :
        spotLight_(light),
        shadowMapSize_(size),
        shadowMapRT_(nullptr),
        blurredShadowMap_(nullptr),
        smProgram_(smProgram),
        filterProgram_(filterProgram),
        filterUniformIds_(filterProgram->GetUniformLocations({ "sourceTex", "targetTex", "dir", "bloomWidth" }))
    {
        assert(components <= 4);
        smProgram->BindUniformBlock(perspectiveProjectionUBBName, *app->GetUBOBindingPoints());

        FrameBufferDescriptor fbd;
        switch (components) {
        case 1: fbd.texDesc.emplace_back(TextureDescriptor{ 32, GL_R32F, GL_RED, GL_FLOAT }); break;
        case 2: fbd.texDesc.emplace_back(TextureDescriptor{ 64, GL_RG32F, GL_RG, GL_FLOAT }); break;
        case 3: fbd.texDesc.emplace_back(TextureDescriptor{ 96, GL_RGB32F, GL_RGB, GL_FLOAT }); break;
        case 4: fbd.texDesc.emplace_back(TextureDescriptor{ 128, GL_RGBA32F, GL_RGBA, GL_FLOAT }); break;
        }
        fbd.rbDesc.emplace_back(RenderBufferDescriptor{ GL_DEPTH_COMPONENT32F });
        shadowMapRT_.reset(new GLRenderTarget(size.x, size.y, fbd));

        blurredShadowMap_ = std::make_unique<GLTexture>(size.x, size.y, fbd.texDesc[0].texDesc_, nullptr);
    }

    /**
         *  Constructor.
         *  @param size the size of the shadow map.
         *  @param app the application base object.
         */
    ShadowMap::ShadowMap(const glm::uvec2& size, const SpotLight& light, ApplicationBase* app) :
        ShadowMap(size, 2, light, app->GetGPUProgramManager()->GetResource("shader/shadowMap.vp|shader/shadowMap.fp"),
        app->GetGPUProgramManager()->GetResource("shader/gaussianFilter.cp,TEX_FORMAT rg32f"), app)
    {
    }

    ShadowMap::~ShadowMap() = default;

    void ShadowMap::RenderShadowGeometry(std::function<void(GLBatchRenderTarget&) > batch)
    {
        glm::vec4 bgValue(spotLight_.GetCamera().GetFarZ(), spotLight_.GetCamera().GetFarZ() * spotLight_.GetCamera().GetFarZ(), 0.0f, 0.0f);
        shadowMapRT_->GetTextures()[0]->SampleWrapBorderColor(bgValue);
        shadowMapRT_->BatchDraw([&](cgu::GLBatchRenderTarget & brt) {
            brt.Clear(static_cast<unsigned int>(cgu::ClearFlags::CF_RenderTarget) | static_cast<unsigned int>(cgu::ClearFlags::CF_Depth), glm::value_ptr(bgValue), 1.0, 0);
            spotLight_.GetCamera().SetViewShadowMap();
            batch(brt);
        });
    }

    void ShadowMap::BlurShadowMap()
    {
        const glm::vec2 groupSize{ 32.0f, 16.0f };
        auto numGroups = glm::ivec2(glm::ceil(glm::vec2(shadowMapSize_) / groupSize));

        filterProgram_->UseProgram();
        filterProgram_->SetUniform(filterUniformIds_[0], 0);
        filterProgram_->SetUniform(filterUniformIds_[1], 0);
        filterProgram_->SetUniform(filterUniformIds_[2], glm::vec2(1.0f, 0.0f));
        filterProgram_->SetUniform(filterUniformIds_[3], 3.5f);
        shadowMapRT_->GetTextures()[0]->ActivateTexture(GL_TEXTURE0);
        blurredShadowMap_->ActivateImage(0, 0, GL_WRITE_ONLY);
        OGL_CALL(glDispatchCompute, numGroups.x, numGroups.y, 1);
        OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
        OGL_SCALL(glFinish);

        filterProgram_->SetUniform(filterUniformIds_[2], glm::vec2(0.0f, 1.0f));
        blurredShadowMap_->ActivateTexture(GL_TEXTURE0);
        shadowMapRT_->GetTextures()[0]->ActivateImage(0, 0, GL_WRITE_ONLY);
        OGL_CALL(glDispatchCompute, numGroups.x, numGroups.y, 1);
        OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
        OGL_SCALL(glFinish);
    }

    void ShadowMap::Resize(const glm::uvec2& smSize)
    {
        shadowMapSize_ = smSize;
        shadowMapRT_->Resize(shadowMapSize_.x, shadowMapSize_.y);
        blurredShadowMap_ = std::make_unique<GLTexture>(smSize.x, smSize.y, shadowMapRT_->GetTextures()[0]->GetDescriptor(), nullptr);
    }

    glm::mat4 ShadowMap::GetViewProjectionTextureMatrix(const glm::mat4& view, const glm::mat4& projection)
    {
        auto translate = glm::translate(glm::mat4(), glm::vec3(0.5f, 0.5f, 0.5f));
        auto bias = glm::scale(translate, glm::vec3(0.5f, 0.5f, 0.5f));
        return bias * projection * view;
    }

    const GLTexture* ShadowMap::GetShadowTexture() const
    {
        return shadowMapRT_->GetTextures()[0].get();
    }
}
