/**
 * @file   RotationAnimation.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.08.29
 *
 * @brief  Declaration of a rotation animation.
 */

#ifndef ROTATIONANIMATION_H
#define ROTATIONANIMATION_H

#include "BaseAnimation.h"
#include <glm/gtc/quaternion.hpp>
#include <core/Arcball.h>
// ReSharper disable CppUnusedIncludeDirective
#include <core/serializationHelper.h>
// ReSharper restore CppUnusedIncludeDirective

namespace cgu {

    class GLWindow;
    class RotationAnimationEditor;
    class PerspectiveCamera;

    class RotationAnimation : public BaseAnimation
    {
    public:
        using Editor = RotationAnimationEditor;

        RotationAnimation();
        RotationAnimation(const glm::quat& startOrientation, const glm::vec3& axis, float frequency);
        ~RotationAnimation();

        const glm::quat& GetStartOrientation() const { return startOrientation_; }
        void SetStartOrientation(const glm::quat& startOrientation) { startOrientation_ = startOrientation; }
        const glm::vec3& GetRotationAxis() const { return rotationAxis_; }
        void SetRotationAxis(const glm::vec3& rotationAxis) { rotationAxis_ = rotationAxis; }
        float GetFrequency() const { return frequency_; }
        void SetFrequency(float frequency) { frequency_ = frequency; }
        const glm::quat& GetCurrentState() const { return currentState_; }

        bool DoAnimationStep(float elapsedTime) override;
        void ShowEditDialog(const std::string& name);

    private:
        /** Needed for serialization */
        friend class boost::serialization::access;

        template<class Archive>
        void save(Archive & ar, const unsigned int) const
        {
            ar & boost::serialization::make_nvp("BaseAnimation", boost::serialization::base_object<BaseAnimation>(*this));
            ar & BOOST_SERIALIZATION_NVP(startOrientation_);
            ar & BOOST_SERIALIZATION_NVP(rotationAxis_);
            ar & BOOST_SERIALIZATION_NVP(frequency_);
        }

        template<class Archive>
        void load(Archive & ar, const unsigned int)
        {
            ar & boost::serialization::make_nvp("BaseAnimation", boost::serialization::base_object<BaseAnimation>(*this));
            ar & BOOST_SERIALIZATION_NVP(startOrientation_);
            ar & BOOST_SERIALIZATION_NVP(rotationAxis_);
            ar & BOOST_SERIALIZATION_NVP(frequency_);
        }

        BOOST_SERIALIZATION_SPLIT_MEMBER()

        /** Holds the start orientation. */
        glm::quat startOrientation_;
        /** Holds the rotation axis. */
        glm::vec3 rotationAxis_;
        /** Holds the frequency. */
        float frequency_;
        /** Holds the current animation state. */
        glm::quat currentState_;
    };

    class RotationAnimationEditor
    {
    public:
        explicit RotationAnimationEditor(const PerspectiveCamera* camera);

        void SetCurrentEdited(RotationAnimation* edit) { edit_ = edit; }
        bool HandleMouse(int button, int action, int mods, float mouseWheelDelta, GLWindow* sender);
        bool HandleKeyboard(int key, int scancode, int action, int mods, GLWindow* sender);

    private:
        /** Holds the edited object. */
        RotationAnimation* edit_ = nullptr;
        /** Holds the camera object. */
        const PerspectiveCamera* camera_;
        /** Holds the arcball used for rotation of the axis. */
        Arcball axisArcball;
    };
}

BOOST_CLASS_VERSION(cgu::RotationAnimation, 1)

#endif // ROTATIONANIMATION_H
