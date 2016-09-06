/**
 * @file   WaypointAnimation.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.08.25
 *
 * @brief  
 */

#include "WaypointAnimation.h"
#include "glm/gtx/spline.hpp"
#include <imgui.h>
#include <GLFW/glfw3.h>

namespace cgu {

    WaypointAnimation::WaypointAnimation() :
        totalTime_(1.0f),
        interpolationMode_(0),
        normalizeTime_(false)
    {
    }

    WaypointAnimation::WaypointAnimation(float totalTime, int interpolationMode, bool normalizeTime) :
        totalTime_(totalTime),
        interpolationMode_(interpolationMode),
        normalizeTime_(normalizeTime)
    {
    }

    WaypointAnimation::~WaypointAnimation() = default;

    void WaypointAnimation::AddWaypoint(const WaypointInfo& wp)
    {
        waypoints_.push_back(std::make_pair(wp, 0.0f));
    }

    void WaypointAnimation::ResetWaypoints()
    {
        waypoints_.clear();
    }

    void WaypointAnimation::StartAnimation()
    {
        BaseAnimation::StartAnimation();
        if (waypoints_.size() < 2) {
            StopAnimation();
            return;
        }
        std::vector<float> distances(waypoints_.size(), totalTime_ / static_cast<float>(waypoints_.size() - 1));
        distances[0] = 0.0f;
        if (normalizeTime_) {
            auto totalDistances = 0.0f;
            for (auto i = 1; i < distances.size(); ++i) {
                distances[i] = glm::distance(waypoints_[i - 1].first.position, waypoints_[i].first.position);
                totalDistances += distances[i];
            }

            for (auto i = 1; i < distances.size(); ++i) distances[i] = distances[i] * totalTime_ / totalDistances;
        }

        waypoints_[0].second = 0.0f;
        for (auto i = 1; i < distances.size(); ++i) waypoints_[i].second = waypoints_[i - 1].second + distances[i];
    }

    bool WaypointAnimation::DoAnimationStep(float elapsedTime)
    {
        if (!BaseAnimation::DoAnimationStep(elapsedTime)) return false;

        auto cI = 1;
        for (; cI < waypoints_.size(); ++cI) if (waypoints_[cI].second > GetCurrentTime()) break;

        if (cI >= waypoints_.size()) {
            cI = static_cast<int>(waypoints_.size() - 1);
            StopAnimation();
        }
        auto alpha = (GetCurrentTime() - waypoints_[cI - 1].second) / (waypoints_[cI].second - waypoints_[cI - 1].second);
        switch (interpolationMode_) {
        case 1: {
            auto v0 = (cI == 1) ? 2.0f * waypoints_[cI - 1].first.position - waypoints_[cI].first.position : waypoints_[cI - 2].first.position;
            auto v4 = (cI == waypoints_.size() - 1) ? 2.0f * waypoints_[cI].first.position - waypoints_[cI - 1].first.position : waypoints_[cI + 1].first.position;
            currentState_.position = glm::catmullRom(v0, waypoints_[cI - 1].first.position, waypoints_[cI].first.position, v4, alpha);
        } break;
        case 2: {
            auto v0 = (cI == 1) ? 2.0f * waypoints_[cI - 1].first.position - waypoints_[cI].first.position : waypoints_[cI - 2].first.position;
            auto v4 = (cI == waypoints_.size() - 1) ? 2.0f * waypoints_[cI].first.position - waypoints_[cI - 1].first.position : waypoints_[cI + 1].first.position;
            currentState_.position = glm::cubic(v0, waypoints_[cI - 1].first.position, waypoints_[cI].first.position, v4, alpha);
        } break;
        case 3: {
            auto t0 = (cI == 1) ? glm::normalize(waypoints_[cI].first.position - waypoints_[cI - 1].first.position)
                : glm::normalize(waypoints_[cI].first.position - waypoints_[cI - 2].first.position);
            auto t1 = (cI == waypoints_.size() - 1) ? glm::normalize(waypoints_[cI].first.position - waypoints_[cI - 1].first.position)
                : glm::normalize(waypoints_[cI + 1].first.position - waypoints_[cI - 1].first.position);
            currentState_.position = glm::hermite(waypoints_[cI - 1].first.position, t0, waypoints_[cI].first.position, t1, alpha);
        } break;
        default: {
            currentState_.position = glm::mix(waypoints_[cI - 1].first.position, waypoints_[cI].first.position, alpha);
        } break;
        }


        if (GetCurrentTime() > waypoints_.back().second) StopAnimation();
        return true;
    }

    void WaypointAnimation::ShowEditDialog(const std::string& name)
    {
        const auto winWidth = 250.0f;
        const auto winHeight = 170.0f;
        ImGui::SetNextWindowSize(ImVec2(winWidth, winHeight), ImGuiSetCond_Always);
        ImGui::SetNextWindowPos(ImVec2(10.0f, ImGui::GetIO().DisplaySize.y - winHeight - 10.0f), ImGuiSetCond_Always);
        ImGui::Begin(("Wayppoint Animation (" + name + ")").c_str());
        ImGui::InputFloat("Total Time", &totalTime_);
        ImGui::RadioButton("Linear Interpolation", &interpolationMode_, 0);
        ImGui::RadioButton("Catmull-Rom Interpolation", &interpolationMode_, 1);
        ImGui::RadioButton("Cubic Interpolation", &interpolationMode_, 2);
        ImGui::RadioButton("Hermite Interpolation", &interpolationMode_, 3);
        ImGui::Checkbox("Normalize Time", &normalizeTime_);
        ImGui::End();
    }

    WaypointAnimationEditor::WaypointAnimationEditor(const PerspectiveCamera*)
    {
    }

    // ReSharper disable once CppMemberFunctionMayBeStatic
    // ReSharper disable once CppMemberFunctionMayBeConst
    bool WaypointAnimationEditor::HandleMouse(int, int, int, float, GLWindow*)
    {
        return false;
    }

    // ReSharper disable once CppMemberFunctionMayBeStatic
    // ReSharper disable once CppMemberFunctionMayBeConst
    bool WaypointAnimationEditor::HandleKeyboard(int key, int, int action, int, GLWindow*)
    {
        return false;
    }
}
