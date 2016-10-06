/**
 * @file   Image2DStatistics.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.09.18
 *
 * @brief  Implements a class to create statistics of 2D images.
 */

#include "Image2DStatistics.h"
#include "gfx/glrenderer/GLTexture2D.h"
#include "gfx/glrenderer/GLTexture.h"
#include "app/ApplicationBase.h"
#include <boost/assign.hpp>
#include <glm/gtx/log_base.hpp>
#undef max
#undef min

namespace cgu {
    namespace eval {

        Image2DStatistics::Image2DStatistics(const std::string& origImage, ApplicationBase* app) :
            origTex_(new GLTexture2D(origImage, app)),
            diffProgram_(app->GetGPUProgramManager()->GetResource("shader/eval/diffImage.cp")),
            diffUniformIds_(diffProgram_->GetUniformLocations({ "origTex", "cmpTex", "resultTex", "statsTex" })),
            reduceProgram_(app->GetGPUProgramManager()->GetResource("shader/eval/reductionRMS.cp")),
            reduceUniformIds_(reduceProgram_->GetUniformLocations({ "reduceTex" })),
            application_(app)
        {
            auto diffTexDesc = origTex_->GetTexture()->GetDescriptor();
            assert(diffTexDesc.bytesPP == 4);
            assert(diffTexDesc.format == GL_RGBA || diffTexDesc.format == GL_BGRA);
            imgDimensions_ = origTex_->GetTexture()->GetDimensions();
            diffTex_.reset(new GLTexture(imgDimensions_.x, imgDimensions_.y, diffTexDesc, nullptr));

            TextureDescriptor statsTexDesc{ 16 , GL_RGBA32F, GL_RGBA, GL_FLOAT };
            statsTex_.reset(new GLTexture(imgDimensions_.x, imgDimensions_.y, statsTexDesc, nullptr));
        }


        Image2DStatistics::~Image2DStatistics() = default;

        EvalStatistics Image2DStatistics::CreateDiffImage(const std::string& compareImage, const std::string& diffImage) const
        {
            const auto workGroupSize = glm::vec2(32.0f, 16.0f);
            auto compareTex = std::make_unique<GLTexture2D>(compareImage, application_);
            diffProgram_->UseProgram();
            diffProgram_->SetUniform(diffUniformIds_[0], 0);
            diffProgram_->SetUniform(diffUniformIds_[1], 1);
            diffProgram_->SetUniform(diffUniformIds_[2], 2);
            diffProgram_->SetUniform(diffUniformIds_[3], 3);
            auto numGroups = glm::ivec2(glm::ceil(glm::vec2(imgDimensions_) / workGroupSize));
            origTex_->GetTexture()->ActivateImage(0, 0, GL_READ_ONLY);
            compareTex->GetTexture()->ActivateImage(1, 0, GL_READ_ONLY);
            diffTex_->ActivateImage(2, 0, GL_WRITE_ONLY);
            statsTex_->ActivateImage(3, 0, GL_WRITE_ONLY);
            OGL_CALL(glDispatchCompute, numGroups.x, numGroups.y, 1);
            OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
            OGL_SCALL(glFinish);

            diffTex_->SaveTextureToFile(application_->GetConfig().evalDirectory + "/" + diffImage);

            while (numGroups.x > 1 || numGroups.y > 1) {
                reduceProgram_->UseProgram();
                reduceProgram_->SetUniform(reduceUniformIds_[0], 0);
                statsTex_->ActivateImage(0, 0, GL_READ_WRITE);
                OGL_CALL(glDispatchCompute, numGroups.x, numGroups.y, 1);
                OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
                OGL_SCALL(glFinish);
                numGroups = glm::ivec2(glm::ceil(glm::vec2(numGroups) / workGroupSize));
            }

            OGL_CALL(glUseProgram, 0);

            std::vector<uint8_t> statResults;
            statsTex_->DownloadData(statResults, 0, 16);
            auto statResultsFlt = reinterpret_cast<float*>(statResults.data());
            auto numPixels = static_cast<float>(imgDimensions_.x * imgDimensions_.y);
            EvalStatistics result;
            result.errorMax_ = statResultsFlt[2];
            result.numErrorPixels_ = statResultsFlt[3];
            result.errorRMSAvg_ = glm::sqrt(statResultsFlt[0] / result.numErrorPixels_);
            result.errorRMSAvgAll_ = glm::sqrt(statResultsFlt[0] / numPixels);
            result.errorRMSMax_ = glm::sqrt(statResultsFlt[1] / result.numErrorPixels_);
            result.errorRMSMaxAll_ = glm::sqrt(statResultsFlt[1] / numPixels);
            result.psnrAvg_ = 20.0f * glm::log(1.0f / result.errorRMSAvg_, 10.0f);
            result.psnrAvgAll_ = 20.0f * glm::log(1.0f / result.errorRMSAvgAll_, 10.0f);
            result.psnrMax_ = 20.0f * glm::log(1.0f / result.errorRMSMax_, 10.0f);
            result.psnrMaxAll_ = 20.0f * glm::log(1.0f / result.errorRMSMaxAll_, 10.0f);
            return result;
        }
    }
}
