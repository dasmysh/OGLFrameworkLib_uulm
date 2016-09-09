/**
 * @file   RotationAnimation.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.08.29
 *
 * @brief  Implementation of a rotation animation.
 */

#include "RotationAnimation.h"
#include <imgui.h>
#include <glm/gtx/quaternion.hpp>
#include <GLFW/glfw3.h>
#include "gfx/PerspectiveCamera.h"

namespace cgu {

    RotationAnimation::RotationAnimation() :
        startOrientation_(),
        rotationAxis_(0.0f, 1.0f, 0.0f),
        frequency_(0.0f)
    {
    }

    RotationAnimation::RotationAnimation(const glm::quat& startOrientation, const glm::vec3& axis, float frequency) :
        startOrientation_(startOrientation),
        rotationAxis_(axis),
        frequency_(frequency)
    {
    }

    RotationAnimation::~RotationAnimation() = default;

    bool RotationAnimation::DoAnimationStep(float elapsedTime)
    {
        if (!BaseAnimation::DoAnimationStep(elapsedTime)) return false;
        if (frequency_ < 0.00001f) {
            StopAnimation();
            return false;
        }
        currentState_ = glm::rotate(startOrientation_, glm::two_pi<float>() * (GetCurrentTime() / frequency_), rotationAxis_);
        return true;
    }

    void RotationAnimation::ShowEditDialog(const std::string& name)
    {
        const auto winWidth = 250.0f;
        const auto winHeight = 80.0f;
        ImGui::SetNextWindowSize(ImVec2(winWidth, winHeight), ImGuiSetCond_Always);
        ImGui::SetNextWindowPos(ImVec2(10.0f, ImGui::GetIO().DisplaySize.y - winHeight - 10.0f), ImGuiSetCond_Always);
        ImGui::Begin(("Rotation Animation (" + name + ")").c_str());
        ImGui::InputFloat("Frequency", &frequency_);
        ImGui::End();
    }

    RotationAnimationEditor::RotationAnimationEditor(const PerspectiveCamera* camera) :
        camera_{ camera },
        axisArcball{ GLFW_MOUSE_BUTTON_LEFT }
    {
    }

    bool RotationAnimationEditor::HandleMouse(int button, int action, int mods, float, GLWindow* sender)
    {
        if (edit_ != nullptr) {
            if (axisArcball.HandleMouse(button, action, mods, sender)) {
                auto orientation = glm::rotation(glm::vec3(0.0f, 1.0f, 0.0f), edit_->GetRotationAxis());
                auto orient = glm::inverse(axisArcball.GetWorldRotation(camera_->GetViewMatrix())) * orientation;
                edit_->SetRotationAxis(glm::rotate(orient, glm::vec3(0.0f, 1.0f, 0.0f)));
                return true;
            }
        }
        return false;
    }

    // ReSharper disable once CppMemberFunctionMayBeStatic
    // ReSharper disable once CppMemberFunctionMayBeConst
    bool RotationAnimationEditor::HandleKeyboard(int, int, int, int, GLWindow*)
    {
        return false;
    }
}
