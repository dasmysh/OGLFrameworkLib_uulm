/**
 * @file   GaussianBlur.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.05.05
 *
 * @brief  Implementation of a Gaussian blur filter.
 */

#define GLM_SWIZZLE
#include "GaussianBlur.h"
#include <app/ApplicationBase.h>

namespace cgu {

    GaussianBlur::GaussianBlur(const GLTexture* source, const std::string& texFormat, const std::string& blurColorType, const std::string& blurSwizzle, ApplicationBase* app) :
        source_{ source },
        size_{ 0 },
        gaussianProgram_(app->GetGPUProgramManager()->GetResource("shader/gaussianFilter.cp,TEX_FORMAT " + texFormat + ",BTYPE " + blurColorType + ",COMP_SWIZZLE " + blurSwizzle)),
        gaussianUniformIds_(gaussianProgram_->GetUniformLocations({ "sourceTex", "targetTex", "dir", "bloomWidth" }))
    {
        Resize();
    }


    GaussianBlur::~GaussianBlur() = default;

    void GaussianBlur::ApplyBlur()
    {
        const glm::vec2 groupSize{ 32.0f, 16.0f };
        auto numGroups = glm::ivec2(glm::ceil(glm::vec2(size_) / groupSize));

        gaussianProgram_->UseProgram();
        gaussianProgram_->SetUniform(gaussianUniformIds_[2], glm::vec2(1.0f, 0.0f));
        gaussianProgram_->SetUniform(gaussianUniformIds_[3], 5.5f);
        std::vector<int> textureStages;

        /*for (auto i = 0; i < blurredShadowMap_.size(); ++i) {
            shadowMapRT_->GetTextures()[i]->ActivateTexture(GL_TEXTURE0 + i);
            blurredShadowMap_[i]->ActivateImage(i, 0, GL_WRITE_ONLY);
            textureStages.push_back(i);
        }
        gaussianProgram_->SetUniform(gaussianUniformIds_[0], textureStages);
        gaussianProgram_->SetUniform(gaussianUniformIds_[1], textureStages);*/
        source_->ActivateTexture(GL_TEXTURE0 + 0);
        tmp_->ActivateImage(0, 0, GL_WRITE_ONLY);
        gaussianProgram_->SetUniform(gaussianUniformIds_[0], 0);
        gaussianProgram_->SetUniform(gaussianUniformIds_[1], 0);
        OGL_CALL(glDispatchCompute, numGroups.x, numGroups.y, 1);
        OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
        OGL_SCALL(glFinish);

        gaussianProgram_->SetUniform(gaussianUniformIds_[2], glm::vec2(0.0f, 1.0f));
        /*for (auto i = 0; i < blurredShadowMap_.size(); ++i) {
            shadowMapRT_->GetTextures()[i]->ActivateImage(i, 0, GL_WRITE_ONLY);
            blurredShadowMap_[i]->ActivateTexture(GL_TEXTURE0 + i);
            }*/
        source_->ActivateImage(0, 0, GL_WRITE_ONLY);
        tmp_->ActivateTexture(GL_TEXTURE0 + 0);
        OGL_CALL(glDispatchCompute, numGroups.x, numGroups.y, 1);
        OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
        OGL_SCALL(glFinish);
    }

    void GaussianBlur::Resize()
    {
        auto texDim = source_->GetDimensions();
        size_.x = texDim.x;
        size_.y = texDim.y;
        tmp_ = std::make_unique<GLTexture>(texDim.x, texDim.y, source_->GetDescriptor(), nullptr);
    }
}
