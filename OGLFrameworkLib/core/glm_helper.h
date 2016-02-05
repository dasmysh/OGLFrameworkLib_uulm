/**
 * @file   glm_helper.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.01.14
 *
 * @brief  Some helper methods for handling glm objects.
 */

#ifndef GLM_HELPER_H
#define GLM_HELPER_H

#include "../main.h"

namespace cgu {
    inline std::ostream& operator<<(std::ostream& str, const glm::vec2& v) {
        str << v.x << "(" << v.y << ")";
        return str;
    }

    inline std::ostream& operator<<(std::ostream& str, const glm::vec3& v) {
        str << v.x << "(" << v.y << ", max: " << v.z << ")";
        return str;
    }
}

#endif // GLM_HELPER_H
