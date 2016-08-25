/**
 * @file   WaypointAnimation.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.08.25
 *
 * @brief  Declaration of the waypoint animation object.
 */

#ifndef WAYPOINTANIMATION_H
#define WAYPOINTANIMATION_H

#include "main.h"
// ReSharper disable CppUnusedIncludeDirective
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include <core/serializationHelper.h>
// ReSharper restore CppUnusedIncludeDirective

namespace cgu {

    struct WaypointInfo
    {
        glm::vec3 position;

        template<class Archive> void serialize(Archive &ar, const unsigned int) {
            ar & BOOST_SERIALIZATION_NVP(position);
        }
    };

    class WaypointAnimation
    {
    public:
        WaypointAnimation();
        ~WaypointAnimation();

        void AddWaypoint(const WaypointInfo& wp);
        void ResetWaypoints();
        void StartAnimation(float totalTime, int interpolationMode, bool normalizeTime);
        void DoAnimationStep(float elapsedTime, WaypointInfo& result);

        bool IsAnimationRunning() const { return animationRunning_; }

    private:
        /** Needed for serialization */
        friend class boost::serialization::access;

        template<class Archive>
        void save(Archive & ar, const unsigned int) const
        {
            ar & BOOST_SERIALIZATION_NVP(waypoints_);
        }

        template<class Archive>
        void load(Archive & ar, const unsigned int)
        {
            ar & BOOST_SERIALIZATION_NVP(waypoints_);
            animationRunning_ = false;
            interpolationMode_ = 0;
            currentAnimationTime_ = 0.0f;
        }

        BOOST_SERIALIZATION_SPLIT_MEMBER()

        /** Holds a list of waypoints. */
        std::vector<std::pair<WaypointInfo, float>> waypoints_;
        /** Hold whether the animation is running. */
        bool animationRunning_ = false;
        /** Holds the interpolation mode. */
        int interpolationMode_ = 0;
        /** Holds the current animation time. */
        float currentAnimationTime_ = 0.0f;
    };
}

BOOST_CLASS_VERSION(cgu::WaypointInfo, 1)
BOOST_CLASS_VERSION(cgu::WaypointAnimation, 1)

#endif // WAYPOINTANIMATION_H
