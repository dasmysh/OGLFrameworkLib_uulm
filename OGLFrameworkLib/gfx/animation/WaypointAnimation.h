/**
 * @file   WaypointAnimation.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.08.25
 *
 * @brief  Declaration of the waypoint animation object.
 */

#ifndef WAYPOINTANIMATION_H
#define WAYPOINTANIMATION_H

#include "BaseAnimation.h"
// ReSharper disable CppUnusedIncludeDirective
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include <core/serializationHelper.h>
// ReSharper restore CppUnusedIncludeDirective

namespace cgu {
    class PerspectiveCamera;

    class GLWindow;
    class WaypointAnimationEditor;

    struct WaypointInfo
    {
        glm::vec3 position;

        template<class Archive> void serialize(Archive &ar, const unsigned int) {
            ar & BOOST_SERIALIZATION_NVP(position);
        }
    };

    class WaypointAnimation : public BaseAnimation
    {
    public:
        using Editor = WaypointAnimationEditor;

        WaypointAnimation();
        WaypointAnimation(float totalTime, int interpolationMode, bool normalizeTime);
        ~WaypointAnimation();

        void AddWaypoint(const WaypointInfo& wp);
        void ResetWaypoints();

        float GetTotalTime() const { return totalTime_; }
        void SetTotalTime(float totalTime) { totalTime_ = totalTime; }
        int GetInterpolationMode() const { return interpolationMode_; }
        void SetInterpolationMode(int interpolationMode) { interpolationMode_ = interpolationMode; }
        bool DoNormalizeTime() const { return normalizeTime_; }
        void SetNormalizeTime(bool normalizeTime) { normalizeTime_ = normalizeTime; }
        const WaypointInfo& GetCurrentState() const { return currentState_; }

        void StartAnimation() override;
        bool DoAnimationStep(float elapsedTime) override;
        void ShowEditDialog(const std::string& name);

    private:
        /** Needed for serialization */
        friend class boost::serialization::access;

        template<class Archive>
        void save(Archive & ar, const unsigned int) const
        {
            ar & boost::serialization::make_nvp("BaseAnimation", boost::serialization::base_object<BaseAnimation>(*this));
            ar & BOOST_SERIALIZATION_NVP(waypoints_);
            ar & BOOST_SERIALIZATION_NVP(totalTime_);
            ar & BOOST_SERIALIZATION_NVP(interpolationMode_);
            ar & BOOST_SERIALIZATION_NVP(normalizeTime_);
        }

        template<class Archive>
        void load(Archive & ar, const unsigned int)
        {
            ar & boost::serialization::make_nvp("BaseAnimation", boost::serialization::base_object<BaseAnimation>(*this));
            ar & BOOST_SERIALIZATION_NVP(waypoints_);
            ar & BOOST_SERIALIZATION_NVP(totalTime_);
            ar & BOOST_SERIALIZATION_NVP(interpolationMode_);
            ar & BOOST_SERIALIZATION_NVP(normalizeTime_);
        }

        BOOST_SERIALIZATION_SPLIT_MEMBER()

        /** Holds a list of waypoints. */
        std::vector<std::pair<WaypointInfo, float>> waypoints_;

        /** Holds the total animation time. */
        float totalTime_;
        /** Holds the interpolation mode. */
        int interpolationMode_;
        /** Holds whether the time needs to be normalized. */
        bool normalizeTime_;
        /** Holds the current animation state. */
        WaypointInfo currentState_;
    };

    class WaypointAnimationEditor
    {
    public:
        explicit WaypointAnimationEditor(const PerspectiveCamera* camera);

        void SetCurrentEdited(WaypointAnimation* edit) { edit_ = edit; }
        bool HandleMouse(int button, int action, int mods, float mouseWheelDelta, GLWindow* sender);
        bool HandleKeyboard(int key, int scancode, int action, int mods, GLWindow* sender);

    private:
        /** Holds the edited object. */
        WaypointAnimation* edit_ = nullptr;
    };
}

BOOST_CLASS_VERSION(cgu::WaypointInfo, 1)
BOOST_CLASS_VERSION(cgu::WaypointAnimation, 2)

#endif // WAYPOINTANIMATION_H
