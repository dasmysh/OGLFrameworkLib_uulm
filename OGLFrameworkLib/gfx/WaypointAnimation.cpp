/**
 * @file   WaypointAnimation.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.08.25
 *
 * @brief  
 */

#include "WaypointAnimation.h"
#include "glm/gtx/spline.hpp"

namespace cgu {

    WaypointAnimation::WaypointAnimation()
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

    void WaypointAnimation::StartAnimation(float totalTime, int interpolationMode, bool normalizeTime)
    {
        std::vector<float> distances(waypoints_.size(), totalTime / static_cast<float>(waypoints_.size() - 1));
        distances[0] = 0.0f;
        if (normalizeTime) {
            auto totalDistances = 0.0f;
            for (auto i = 1; i < distances.size(); ++i) {
                distances[i] = glm::distance(waypoints_[i - 1].first.position, waypoints_[i].first.position);
                totalDistances += distances[i];
            }

            for (auto i = 1; i < distances.size(); ++i) distances[i] = distances[i] * totalTime / totalDistances;
        }

        waypoints_[0].second = 0.0f;
        for (auto i = 1; i < distances.size(); ++i) waypoints_[i].second = waypoints_[i - 1].second + distances[i];
        currentAnimationTime_ = 0.0f;
        interpolationMode_ = interpolationMode;
        animationRunning_ = true;
    }

    void WaypointAnimation::DoAnimationStep(float elapsedTime, WaypointInfo& result)
    {
        if (!animationRunning_) return;

        currentAnimationTime_ += elapsedTime;
        auto cI = 1;
        for (; cI < waypoints_.size(); ++cI) if (waypoints_[cI].second > currentAnimationTime_) break;

        if (cI >= waypoints_.size()) {
            cI = static_cast<int>(waypoints_.size() - 1);
            currentAnimationTime_ = waypoints_[cI].second;
            animationRunning_ = false;
        }
        auto alpha = (currentAnimationTime_ - waypoints_[cI - 1].second) / (waypoints_[cI].second - waypoints_[cI - 1].second);
        switch (interpolationMode_) {
        case 1: {
            auto v0 = (cI == 1) ? 2.0f * waypoints_[cI - 1].first.position - waypoints_[cI].first.position : waypoints_[cI - 2].first.position;
            auto v4 = (cI == waypoints_.size() - 1) ? 2.0f * waypoints_[cI].first.position - waypoints_[cI - 1].first.position : waypoints_[cI + 1].first.position;
            result.position = glm::catmullRom(v0, waypoints_[cI - 1].first.position, waypoints_[cI].first.position, v4, alpha);
        } break;
        case 2: {
            auto v0 = (cI == 1) ? 2.0f * waypoints_[cI - 1].first.position - waypoints_[cI].first.position : waypoints_[cI - 2].first.position;
            auto v4 = (cI == waypoints_.size() - 1) ? 2.0f * waypoints_[cI].first.position - waypoints_[cI - 1].first.position : waypoints_[cI + 1].first.position;
            result.position = glm::cubic(v0, waypoints_[cI - 1].first.position, waypoints_[cI].first.position, v4, alpha);
        } break;
        case 3: {
            auto t0 = (cI == 1) ? glm::normalize(waypoints_[cI].first.position - waypoints_[cI - 1].first.position)
                : glm::normalize(waypoints_[cI].first.position - waypoints_[cI - 2].first.position);
            auto t1 = (cI == waypoints_.size() - 1) ? glm::normalize(waypoints_[cI].first.position - waypoints_[cI - 1].first.position)
                : glm::normalize(waypoints_[cI + 1].first.position - waypoints_[cI - 1].first.position);
            result.position = glm::hermite(waypoints_[cI - 1].first.position, t0, waypoints_[cI].first.position, t1, alpha);
        } break;
        default: {
            result.position = glm::mix(waypoints_[cI - 1].first.position, waypoints_[cI].first.position, alpha);
        } break;
        }


        if (currentAnimationTime_ > waypoints_.back().second) animationRunning_ = false;
    }
}
