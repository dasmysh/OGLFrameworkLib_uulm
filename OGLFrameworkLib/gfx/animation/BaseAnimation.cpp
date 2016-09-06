/**
 * @file   BaseAnimation.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.08.29
 *
 * @brief  Implementation of the animation base class.
 */

#include "BaseAnimation.h"

namespace cgu {
    
    BaseAnimation::BaseAnimation()
    {
    }

    BaseAnimation::~BaseAnimation() = default;

    void BaseAnimation::StartAnimation()
    {
        animationRunning_ = true;
        currentAnimationTime_ = 0.0f;
    }

    bool BaseAnimation::DoAnimationStep(float elapsedTime)
    {
        if (!animationRunning_) return false;
        currentAnimationTime_ += elapsedTime;
        return true;
    }
}
