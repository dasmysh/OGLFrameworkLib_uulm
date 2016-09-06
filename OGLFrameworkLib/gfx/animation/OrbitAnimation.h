/**
 * @file   OrbitAnimation.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.08.29
 *
 * @brief  Declaration of the orbit animation object.
 */

#ifndef ORBITANIMATION_H
#define ORBITANIMATION_H

#include "BaseAnimation.h"
#include <core/Arcball.h>
// ReSharper disable CppUnusedIncludeDirective
#include <core/serializationHelper.h>
// ReSharper restore CppUnusedIncludeDirective

namespace cgu {
    class PerspectiveCamera;

    class GLWindow;
    class OrbitAnimationEditor;

    class OrbitAnimation : public BaseAnimation
    {
    public:
        using Editor = OrbitAnimationEditor;

        OrbitAnimation();
        OrbitAnimation(const glm::vec3& startPosition, const glm::vec3& axis, float frequency);
        ~OrbitAnimation();

        const glm::vec3& GetStartPosition() const { return startPosition_; }
        void SetStartPosition(const glm::vec3& startPosition) { startPosition_ = startPosition; }
        const glm::vec3& GetRotationAxis() const { return rotationAxis_; }
        void SetRotationAxis(const glm::vec3& rotationAxis) { rotationAxis_ = rotationAxis; }
        float GetFrequency() const { return frequency_; }
        void SetFrequency(float frequency) { frequency_ = frequency; }
        const glm::vec3& GetCurrentState() const { return currentState_; }

        bool DoAnimationStep(float elapsedTime) override;
        void ShowEditDialog(const std::string& name);

    private:
        /** Needed for serialization */
        friend class boost::serialization::access;

        template<class Archive>
        void save(Archive & ar, const unsigned int) const
        {
            ar & boost::serialization::make_nvp("BaseAnimation", boost::serialization::base_object<BaseAnimation>(*this));
            ar & BOOST_SERIALIZATION_NVP(startPosition_);
            ar & BOOST_SERIALIZATION_NVP(rotationAxis_);
            ar & BOOST_SERIALIZATION_NVP(frequency_);
        }

        template<class Archive>
        void load(Archive & ar, const unsigned int)
        {
            ar & boost::serialization::make_nvp("BaseAnimation", boost::serialization::base_object<BaseAnimation>(*this));
            ar & BOOST_SERIALIZATION_NVP(startPosition_);
            ar & BOOST_SERIALIZATION_NVP(rotationAxis_);
            ar & BOOST_SERIALIZATION_NVP(frequency_);
        }

        BOOST_SERIALIZATION_SPLIT_MEMBER()

        /** Holds the start position. */
        glm::vec3 startPosition_;
        /** Holds the rotation axis. */
        glm::vec3 rotationAxis_;
        /** Holds the frequency. */
        float frequency_;
        /** Holds the current animation state. */
        glm::vec3 currentState_;
    };

    class OrbitAnimationEditor
    {
    public:
        explicit OrbitAnimationEditor(const PerspectiveCamera* camera);

        void SetCurrentEdited(OrbitAnimation* edit) { edit_ = edit; }
        bool HandleMouse(int button, int action, int mods, float mouseWheelDelta, GLWindow* sender);
        bool HandleKeyboard(int key, int scancode, int action, int mods, GLWindow* sender);

    private:
        /** Holds the edited object. */
        OrbitAnimation* edit_ = nullptr;
        /** Holds the camera object. */
        const PerspectiveCamera* camera_;
        /** Holds the arcball used for rotation of the axis. */
        Arcball axisArcball;
    };
}

BOOST_CLASS_VERSION(cgu::OrbitAnimation, 1)


#endif // ORBITANIMATION_H
