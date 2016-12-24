/**
 * @file   v.h
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2016.12.24
 *
 * @brief  Declaration of the profiling system.
 */

#ifndef PROFILINGSYSTEM_H
#define PROFILINGSYSTEM_H

#include "main.h"
#include <stack>

namespace cgu {

    class ApplicationBase;

    struct ProfilingSection
    {
        ProfilingSection(const std::string& name, double startTime) : name_(name), startTime_(startTime) {}

        /** Holds the sections name. */
        std::string name_;
        /** Holds the section start time. */
        double startTime_;
    };

    class ProfilingSystem
    {
    public:
        void Init(const std::string& evalFilename, ApplicationBase* app, bool newFile = true);

        void StartSection(const std::string& sectionName);
        void EndSection(const std::string& sectionName);

        static ProfilingSystem* instance();

    private:
        ProfilingSystem();
        ~ProfilingSystem();

        /** Holds the systems instance. */
        static std::unique_ptr<ProfilingSystem> instance_;

        /** Holds the evaluation file name. */
        std::string evalFilename_;
        /** Holds the timers ticks per second. */
        double ticksPerSec_;
        /** Holds the section stack. */
        std::stack<ProfilingSection> sectionStack_;
        /** Holds the profiling out stream. */
        std::ofstream profileOut_;

        /** Holds the application object. */
        ApplicationBase* application_;
    };
}

#endif // PROFILINGSYSTEM_H
