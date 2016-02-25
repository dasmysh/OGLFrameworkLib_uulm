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
}

namespace glm {
    inline std::ostream& operator<<(std::ostream& str, const glm::vec3& v){
        return str << "(" << v.x << " " << v.y << " " << v.z << ")";
    }

    inline std::istream& operator>>(std::istream& str, glm::vec3& v){
        char bVec, eVec;
        return str >> bVec >> v.x >> std::ws >> v.y >> std::ws >> v.z >> eVec;
    }

    inline std::ostream& operator<<(std::ostream& str, const glm::vec4& v){
        return str << "(" << v.x << " " << v.y << " " << v.z << " " << v.w << ")";
    }

    inline std::istream& operator>>(std::istream& str, glm::vec4& v){
        char bVec, eVec;
        return str >> bVec >> v.x >> std::ws >> v.y >> std::ws >> v.z >> std::ws >> v.w >> eVec;
    }
}

#endif // GLM_HELPER_H
