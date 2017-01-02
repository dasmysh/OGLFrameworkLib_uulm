/**
 * @file   ProfilingSystem.cpp
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2016.12.24
 *
 * @brief  Implementation of the profiling system.
 */

#include "ProfilingSystem.h"
#include <app/ApplicationBase.h>
#include <fstream>

namespace cgu {

    std::unique_ptr<ProfilingSystem> ProfilingSystem::instance_ = nullptr;

    ProfilingSystem::ProfilingSystem() :
        ticksPerSec_(0.0),
        application_(nullptr)
    {
    }


    ProfilingSystem::~ProfilingSystem()
    {
        profileOut_ << std::endl;
        auto timeEnd = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        auto t = localtime(&timeEnd);

        profileOut_ << "--------------------------------------------------------------------------------" << std::endl;
        profileOut_ << "Ending profiling run (" << std::put_time(t, "%Y/%m/%d %H:%M:%S") << ").";
        profileOut_ << "--------------------------------------------------------------------------------" << std::endl;
        profileOut_.close();
    }


    ProfilingSystem* ProfilingSystem::instance()
    {
        if (!instance_) instance_.reset(new ProfilingSystem());
        return instance_.get();
    }


    void ProfilingSystem::Init(const std::string& evalFilename, ApplicationBase* app, bool newFile /* = true */)
    {
        evalFilename_ = evalFilename;
        application_ = app;

        LARGE_INTEGER qwTicksPerSec = { 0, 0 };
        QueryPerformanceFrequency(&qwTicksPerSec);
        ticksPerSec_ = static_cast<double>(qwTicksPerSec.QuadPart);

        std::ios::openmode mode = std::ios::out;
        if (newFile) mode |= std::ios::trunc;
        else mode |= std::ios::app;
        profileOut_ = std::ofstream(application_->GetConfig().evalDirectory + "/" + evalFilename_, mode);
        if (!newFile) profileOut_ << std::endl << std::endl;

        auto timeBegin = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        auto t = localtime(&timeBegin);

        profileOut_ << "--------------------------------------------------------------------------------" << std::endl;
        profileOut_ << "Starting new profiling run (" << std::put_time(t, "%Y/%m/%d %H:%M:%S") << ") ...";
        profileOut_ << "--------------------------------------------------------------------------------" << std::endl;
        profileOut_.flush();
    }

    void ProfilingSystem::StartSection(const std::string& sectionName)
    {
        LARGE_INTEGER qwTime;
        QueryPerformanceCounter(&qwTime);
        double startTime = static_cast<double>(qwTime.QuadPart);

        sectionStack_.emplace(sectionName, startTime);
        profileOut_ << "Section " << sectionName << "started.";
    }

    void ProfilingSystem::EndSection(const std::string& sectionName)
    {
        const auto stackSectionName = sectionStack_.top().name_;
        const auto stackSectionStartTime = sectionStack_.top().startTime_;
        assert(stackSectionName == sectionName);

        LARGE_INTEGER qwTime;
        QueryPerformanceCounter(&qwTime);
        double endTime = static_cast<double>(qwTime.QuadPart);
        sectionStack_.pop();

        auto duration = (endTime - stackSectionStartTime) / ticksPerSec_;
        profileOut_ << sectionName << ": " << duration;
        profileOut_.flush();
    }
}
