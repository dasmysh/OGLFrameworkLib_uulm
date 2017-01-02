/**
 * @file   ProfilingHelper.h
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2016.12.24
 *
 * @brief  Declaration of a helper class for profiling.
 */

#ifndef PROFILINGHELPER_H
#define PROFILINGHELPER_H

#include "main.h"

namespace cgu {

    class ProfilingHelper
    {
    public:
#ifdef ENABLE_PROFILING
        ProfilingHelper(const std::string& sectionName);
        ~ProfilingHelper();

    private:
        /** Holds the evaluation section name. */
        std::string sectionName_;
#else
        ProfilingHelper(const std::string& sectionName) {};
#endif
    };
}

#define PROFILECAT3(a, c) a##c
#define PROFILE_2(arg, line) cgu::ProfilingHelper PROFILECAT3(_profile, line)((arg))
#define PROFILE(arg)    PROFILE_2((arg), __LINE__)

#endif // PROFILINGHELPER_H
