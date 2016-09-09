/**
 * @file   BaseAnimation.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.08.29
 *
 * @brief  Declaration of the base class for animations.
 */

#ifndef BASEANIMATION_H
#define BASEANIMATION_H

#include "main.h"
// ReSharper disable CppUnusedIncludeDirective
#include <boost/serialization/version.hpp>
#include <boost/serialization/utility.hpp>
// ReSharper restore CppUnusedIncludeDirective

namespace cgu {

    class BaseAnimation
    {
    public:
        BaseAnimation();
        virtual ~BaseAnimation();

        virtual void StartAnimation();
        void StopAnimation() { animationRunning_ = false; }
        virtual bool DoAnimationStep(float elapsedTime);

        bool IsAnimationRunning() const { return animationRunning_; }

    protected:
        float GetCurrentTime() const { return currentAnimationTime_; }

    private:
        /** Needed for serialization */
        friend class boost::serialization::access;

        template<class Archive>
        void save(Archive & ar, const unsigned int) const {}

        template<class Archive>
        void load(Archive & ar, const unsigned int)
        {
            animationRunning_ = false;
            currentAnimationTime_ = 0.0f;
        }

        BOOST_SERIALIZATION_SPLIT_MEMBER()

        /** Hold whether the animation is running. */
        bool animationRunning_ = false;
        /** Holds the current animation time. */
        float currentAnimationTime_ = 0.0f;
    };
}

BOOST_CLASS_VERSION(cgu::BaseAnimation, 1)

#endif // BASEANIMATION_H
