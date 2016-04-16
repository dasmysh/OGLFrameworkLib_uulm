/**
 * @file   Arcball.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.05.28
 *
 * @brief  Definition of the arcball.
 */

#ifndef ARCBALL_H
#define ARCBALL_H

#include <glm/glm.hpp>
namespace cgu {

    class GLWindow;

    /**
    * @brief  Helper class for generic arcballs.
    *
    * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
    * @date   2015.05.28
    */
    class Arcball
    {
    public:
        explicit Arcball(int button);

        bool HandleMouse(int button, int action, int mods, GLWindow* sender);
        glm::quat GetWorldRotation(const glm::mat4& view);

    private:
        static glm::vec3 GetArcballPosition(const glm::vec2 screenPos, const glm::vec2 clientSize);

        /** Holds the action button flag to use. */
        int button_;
        /** Holds whether the arcball is currently rotated. */
        bool arcballOn_;
        /** holds the current arcball position. */
        glm::vec3 currentArcballPos_;
        /** holds the last arcball position. */
        glm::vec3 lastArcballPos_;
    };
}

#endif /* ARCBALL_H */
