/**
 * @file   ShadowMap.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.01.08
 *
 * @brief  Implementation of the shadow map class.
 */

#define GLM_SWIZZLE
#include "ShadowMap.h"
#include "app/ApplicationBase.h"
#include "gfx/glrenderer/GLRenderTarget.h"
#include "SpotLight.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "gfx/glrenderer/GLTexture.h"

namespace cgu {

    ShadowMap::ShadowMap(std::unique_ptr<GLRenderTarget>&& shadowMapRT, const SpotLight& light, const std::shared_ptr<GPUProgram>& smProgram,
        const std::shared_ptr<GPUProgram>& filterProgram, ApplicationBase* app) :
        spotLight_(light),
        shadowMapSize_(0),
        shadowMapRT_(std::move(shadowMapRT)),
        blurredShadowMap_(shadowMapRT_->GetTextures().size()),
        smProgram_(smProgram),
        filterProgram_(filterProgram),
        filterUniformIds_(filterProgram_->GetUniformLocations({ "sourceTex", "targetTex", "dir", "bloomWidth" }))
    {
        smProgram->BindUniformBlock(perspectiveProjectionUBBName, *app->GetUBOBindingPoints());
        shadowMapSize_ = shadowMapRT_->GetTextures()[0]->GetDimensions().xy();
        CreateBlurredTargets();
    }

    static std::unique_ptr<GLRenderTarget> createStandardRT(const glm::uvec2& size)
    {
        FrameBufferDescriptor fbDesc;
        fbDesc.texDesc_.emplace_back(TextureDescriptor{ 64, GL_RG32F, GL_RG, GL_FLOAT });
        return std::make_unique<GLRenderTarget>(size.x, size.y, fbDesc);
    }

    /**
     *  Constructor.
     *  @param size the size of the shadow map.
     *  @param app the application base object.
     */
    ShadowMap::ShadowMap(const glm::uvec2& size, const SpotLight& light, ApplicationBase* app) :
        ShadowMap(createStandardRT(size), light, app->GetGPUProgramManager()->GetResource("shader/shadowMap.vp|shader/shadowMap.fp"),
        app->GetGPUProgramManager()->GetResource("shader/gaussianFilter.cp,TEX_FORMAT rg32f"), app)
    {
    }

    ShadowMap::~ShadowMap() = default;

    void ShadowMap::RenderShadowGeometry(const glm::vec4& clearColor, std::function<void(GLBatchRenderTarget&) > batch)
    {
        // glm::vec4 bgValue(spotLight_.GetCamera().GetFarZ(), spotLight_.GetCamera().GetFarZ() * spotLight_.GetCamera().GetFarZ(), 0.0f, 0.0f);
        shadowMapRT_->GetTextures()[0]->SampleWrapBorderColor(clearColor);
        shadowMapRT_->BatchDraw([&](cgu::GLBatchRenderTarget & brt) {
            brt.Clear(static_cast<unsigned int>(cgu::ClearFlags::CF_RenderTarget) | static_cast<unsigned int>(cgu::ClearFlags::CF_Depth), glm::value_ptr(clearColor), clearColor.x, 0);
            spotLight_.GetCamera().SetViewShadowMap();
            batch(brt);
        });
    }

    void ShadowMap::BlurShadowMap()
    {
        const glm::vec2 groupSize{ 32.0f, 16.0f };
        auto numGroups = glm::ivec2(glm::ceil(glm::vec2(shadowMapSize_) / groupSize));

        filterProgram_->UseProgram();
        filterProgram_->SetUniform(filterUniformIds_[2], glm::vec2(1.0f, 0.0f));
        filterProgram_->SetUniform(filterUniformIds_[3], 3.5f);
        std::vector<int> textureStages;
        for (auto i = 0; i < blurredShadowMap_.size(); ++i) {
            shadowMapRT_->GetTextures()[i]->ActivateTexture(GL_TEXTURE0 + i);
            blurredShadowMap_[i]->ActivateImage(i, 0, GL_WRITE_ONLY);
            textureStages.push_back(i);
        }
        filterProgram_->SetUniform(filterUniformIds_[0], textureStages);
        filterProgram_->SetUniform(filterUniformIds_[1], textureStages);
        OGL_CALL(glDispatchCompute, numGroups.x, numGroups.y, 1);
        OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
        OGL_SCALL(glFinish);

        filterProgram_->SetUniform(filterUniformIds_[2], glm::vec2(0.0f, 1.0f));
        for (auto i = 0; i < blurredShadowMap_.size(); ++i) {
            shadowMapRT_->GetTextures()[i]->ActivateImage(i, 0, GL_WRITE_ONLY);
            blurredShadowMap_[i]->ActivateTexture(GL_TEXTURE0 + i);
        }
        OGL_CALL(glDispatchCompute, numGroups.x, numGroups.y, 1);
        OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
        OGL_SCALL(glFinish);
    }

    void ShadowMap::Resize(const glm::uvec2& smSize)
    {
        shadowMapSize_ = smSize;
        shadowMapRT_->Resize(shadowMapSize_.x, shadowMapSize_.y);
        CreateBlurredTargets();
    }

    glm::mat4 ShadowMap::GetViewProjectionTextureMatrix(const glm::mat4& view, const glm::mat4& projection)
    {
        auto translate = glm::translate(glm::mat4(), glm::vec3(0.5f, 0.5f, 0.5f));
        auto bias = glm::scale(translate, glm::vec3(0.5f, 0.5f, 0.5f));
        return bias * projection * view;
    }

    /*const GLTexture* ShadowMap::GetShadowTexture() const
    {
        return shadowMapRT_->GetTextures()[0].get();
    }*/

    void ShadowMap::CreateBlurredTargets()
    {
        unsigned int i = 0;
        for (const auto& rtTex : shadowMapRT_->GetTextures()) {
            auto texDim = rtTex->GetDimensions();
            assert(texDim.x == shadowMapSize_.x);
            assert(texDim.y == shadowMapSize_.y);
            assert(texDim.z == 1);
            blurredShadowMap_[i++] = std::make_unique<GLTexture>(texDim.x, texDim.y, rtTex->GetDescriptor(), nullptr);
        }
    }
}
