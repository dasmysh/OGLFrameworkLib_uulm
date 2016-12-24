/**
 * @file   ProfilingHelper.cpp
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2016.12.24
 *
 * @brief  Implementation of helper class for profiling.
 */

#include "ProfilingHelper.h"
#include "ProfilingSystem.h"

namespace cgu {

#ifdef ENABLE_PROFILING
    ProfilingHelper::ProfilingHelper(const std::string& sectionName) :
        sectionName_(sectionName)
    {
        ProfilingSystem::instance()->StartSection(sectionName_);
    }


    ProfilingHelper::~ProfilingHelper()
    {
        ProfilingSystem::instance()->EndSection(sectionName_);
    }
#endif
}
