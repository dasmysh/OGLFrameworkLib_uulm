/**
 * @file   Arcball.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.05.28
 *
 * @brief  Implementation of the arcball.
 */

#include "Arcball.h"
#include "app/GLWindow.h"
#include <glm/gtc/quaternion.hpp>
#include <GLFW/glfw3.h>

#undef min
#undef max

namespace cgu {

    Arcball::Arcball(int button) :
        button_(button),
        arcballOn_(false),
        currentArcballPos_(0.0f),
        lastArcballPos_(0.0f)
    {

    }

    bool Arcball::HandleMouse(int button, int action, int, GLWindow* sender)
    {
        auto handled = false;
        if (button_ == button && action == GLFW_PRESS) {
            arcballOn_ = true;
            lastArcballPos_ = currentArcballPos_ = GetArcballPosition(sender->GetMousePosition(), sender->GetClientSize());
            handled = true;
        } else if (arcballOn_ && sender->IsMouseButtonPressed(button_)) {
            currentArcballPos_ = GetArcballPosition(sender->GetMousePosition(), sender->GetClientSize());
            handled = true;
        } else if (!sender->IsMouseButtonPressed(button_)) {
            handled = arcballOn_;
            arcballOn_ = false;
        }

        return handled;
    }


    glm::quat Arcball::GetWorldRotation(const glm::mat4& view)
    {
        glm::quat result(1.0f, 0.0f, 0.0f, 0.0f);
        if (currentArcballPos_ != lastArcballPos_) {
            auto angle = acos(glm::min(1.0f, glm::dot(lastArcballPos_, currentArcballPos_)));
            auto camAxis = glm::cross(lastArcballPos_, currentArcballPos_);
            auto worldAxis = glm::normalize(glm::vec3(glm::inverse(glm::mat3(view)) * camAxis));
            result = glm::angleAxis(-1.5f * angle, worldAxis);
            lastArcballPos_ = currentArcballPos_;
        }
        return result;
    }

    glm::vec3 Arcball::GetArcballPosition(const glm::vec2 screenPos, const glm::vec2 clientSize)
    {
        glm::vec3 result((2.0f * screenPos.x - clientSize.x) / clientSize.x,
            -(2.0f * screenPos.y - clientSize.y) / clientSize.y, 0.0f);
        result = glm::clamp(result, glm::vec3(-1.0f), glm::vec3(1.0f));

        auto length_squared = glm::dot(result, result);
        if (length_squared <= 1.0f)
            result.z = sqrtf(1.0f - length_squared);
        else
            result = glm::normalize(result);

        return result;
    }
}
