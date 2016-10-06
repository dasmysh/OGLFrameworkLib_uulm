/**
 * @file   ScreenCaptureHelper.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.09.06
 *
 * @brief  Implementation of a helper class for screen captures.
 */

#include "ScreenCaptureHelper.h"
#include <gfx/glrenderer/FrameBuffer.h>
#include <gfx/glrenderer/GLRenderTarget.h>
#include <gfx/PerspectiveCamera.h>
#include <fstream>
#include <boost/filesystem/operations.hpp>
#include <gpgpu/Image2DStatistics.h>
#include <app/ApplicationBase.h>

namespace cgu {

    ScreenCaptureHelper::ScreenCaptureHelper(const std::string& directory, const glm::uvec2& size, ApplicationBase* app) :
        directory_(directory),
        application_(app)
    {
        boost::filesystem::create_directory(application_->GetConfig().evalDirectory + "/" + directory_);

        cgu::FrameBufferDescriptor scrShotTargetDesc;
        scrShotTargetDesc.texDesc_.push_back(cgu::TextureDescriptor{ 4, GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE });
        scrShotTargetDesc.rbDesc_.push_back(cgu::RenderBufferDescriptor{ GL_DEPTH_COMPONENT32 });

        scrShotTarget_ = std::make_unique<GLRenderTarget>(size.x, size.y, scrShotTargetDesc);
    }

    ScreenCaptureHelper::~ScreenCaptureHelper()
    {
        WriteStatistics();
    }

    void ScreenCaptureHelper::RenderScreenShot(const std::string& name, const PerspectiveCamera& camera, std::function<void(const PerspectiveCamera&, GLRenderTarget&)> drawFn)
    {
        RenderScreenShot(name, 0, name, camera, [drawFn](unsigned, const PerspectiveCamera& cam, GLRenderTarget& rt) {
            drawFn(cam, rt);
        });
    }

    void ScreenCaptureHelper::RenderScreenShot(const std::string& name, unsigned int techniqueId, const std::string& techniqueName, const PerspectiveCamera& camera, std::function<void(unsigned technique, const PerspectiveCamera&, GLRenderTarget&)> drawFn)
    {
        LARGE_INTEGER qwTicksPerSec = { 0, 0 };
        QueryPerformanceFrequency(&qwTicksPerSec);
        auto QPFTicksPerSec = static_cast<double>(qwTicksPerSec.QuadPart);

        LARGE_INTEGER qwTime;
        QueryPerformanceCounter(&qwTime);
        auto timeStart = static_cast<double>(qwTime.QuadPart);

        glEnable(GL_FRAMEBUFFER_SRGB);
        scrShotTarget_->BatchDraw([&](cgu::GLBatchRenderTarget & rt) {
            glDepthMask(GL_TRUE);
            glEnable(GL_DEPTH_TEST);
            glCullFace(GL_BACK);
            glEnable(GL_CULL_FACE);
            float clearColor[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
            rt.Clear(static_cast<unsigned int>(cgu::ClearFlags::CF_RenderTarget) | static_cast<unsigned int>(cgu::ClearFlags::CF_Depth), clearColor, 1.0, 0);
        });

        drawFn(techniqueId, camera, *scrShotTarget_);

        OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
        OGL_CALL(glFlush);
        OGL_CALL(glFinish);

        QueryPerformanceCounter(&qwTime);
        auto timeEnd = static_cast<double>(qwTime.QuadPart);

        auto timePerDraw = static_cast<float>((timeEnd - timeStart) / (QPFTicksPerSec * 10.0f));
        try {
            frameTimes_[techniqueName] = std::make_pair(timePerDraw + frameTimes_.at(techniqueName).first, frameTimes_.at(techniqueName).second + 1.0f);
        }
        catch (...) {
            frameTimes_[techniqueName] = std::make_pair(timePerDraw, 1.0f);
        }
        auto fileName = directory_ + "/" + name;
        fileNames_[techniqueName] = fileName;
        scrShotTarget_->GetTextures()[0]->SaveTextureToFile(application_->GetConfig().evalDirectory + "/" + fileName + ".png");
    }

    void ScreenCaptureHelper::SetupVideo(const std::vector<std::string>& techniqueNames)
    {
        techniqueNames_ = techniqueNames;
    }

    void ScreenCaptureHelper::RenderVideo(const std::string& name, PerspectiveCamera& camera, float duration,
        std::function<void(unsigned technique, const PerspectiveCamera&, GLRenderTarget&)> drawFn,
        std::function<void(PerspectiveCamera&, float, float)> updateFn)
    {
        const auto fps = 30.0f;

        auto steps = static_cast<unsigned>(duration * fps);
        auto stepDigits = static_cast<unsigned>(std::to_string(steps).size() + 1);
        auto currentTime = 0.0f;

        for (auto i = 0U; i < steps; ++i) {
            updateFn(camera, currentTime, 1.0f / fps);

            for (auto techIdx = 0U; techIdx < techniqueNames_.size(); ++techIdx) {
                std::stringstream nameStream;
                nameStream << name << "_" << techniqueNames_[techIdx] << "_" << std::setfill('0') << std::setw(stepDigits) << i;
                auto filename = nameStream.str();

                RenderScreenShot(filename, techIdx, techniqueNames_[techIdx], camera, drawFn);
            }

            currentTime += 1.0f / fps;
        }
    }

    void ScreenCaptureHelper::WriteStatistics()
    {
        const std::string statisticsFilename = "statistics.txt";
        std::ofstream statisticsOut(application_->GetConfig().evalDirectory + "/" + directory_ + "/" + statisticsFilename, std::ios::out);

        try {
            auto gtFileName = fileNames_.at("GT");
            eval::Image2DStatistics imageStats(gtFileName + ".png", application_);
            for (const auto& entry : fileNames_) {
                if (entry.first == "GT") continue;
                auto diffResults = imageStats.CreateDiffImage(entry.second + ".png", entry.second + "_diff.png");
                statisticsOut << entry.first << ":" << std::endl;
                auto frameTime = frameTimes_[entry.first];
                statisticsOut << "Frame Time:                " << frameTime.first / frameTime.second << std::endl;
                statisticsOut << "Max. Error:                " << diffResults.errorMax_ << std::endl;
                statisticsOut << "Num. Error Pixels:         " << diffResults.numErrorPixels_ << std::endl;
                statisticsOut << "RMS Error (Avg,ErrPixles): " << diffResults.errorRMSAvg_ << std::endl;
                statisticsOut << "RMS Error (Avg,All):       " << diffResults.errorRMSAvgAll_ << std::endl;
                statisticsOut << "RMS Error (Max,ErrPixles): " << diffResults.errorRMSMax_ << std::endl;
                statisticsOut << "RMS Error (Max,All):       " << diffResults.errorRMSMaxAll_ << std::endl;
                statisticsOut << "PSNR (Avg,ErrPixels):      " << diffResults.psnrAvg_ << std::endl;
                statisticsOut << "PSNR (Avg,All):            " << diffResults.psnrAvgAll_ << std::endl;
                statisticsOut << "PSNR (Max,ErrPixels):      " << diffResults.psnrMax_ << std::endl;
                statisticsOut << "PSNR (Max,All):            " << diffResults.psnrMaxAll_ << std::endl << std::endl;
            }
        }
        catch (...) {
            for (const auto& entry : frameTimes_) {
                statisticsOut << entry.first << ":" << std::endl;
                statisticsOut << "Frame Time:   " << entry.second.first / entry.second.second << std::endl << std::endl;
            }
        }
    }
}
