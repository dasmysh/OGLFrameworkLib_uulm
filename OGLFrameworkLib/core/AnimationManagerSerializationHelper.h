/**
 * @file   AnimationManagerSerializationHelper.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.08.25
 *
 * @brief  Definition of the serialization helper for the animation manager.
 */

#ifndef ANIMATIONMANAGERSERIALIZATIONHELPER_H
#define ANIMATIONMANAGERSERIALIZATIONHELPER_H

#include "main.h"

namespace cgu {
    class WaypointAnimation;

    class AnimationManagerSerializationHelper final
    {
    public:
        static void LoadAnimation(std::istream& file, WaypointAnimation& animation);
        static void SaveAnimation(std::ostream& file, const WaypointAnimation& animation);
        static void LoadAnimations(std::istream& file, std::vector<WaypointAnimation>& animations, std::vector<std::string>& animationNames);
        static void SaveAnimations(std::ostream& file, const std::vector<WaypointAnimation>& animations, const std::vector<std::string>& animationNames);
    private:
        AnimationManagerSerializationHelper();
        ~AnimationManagerSerializationHelper();
    };
}

#endif // ANIMATIONMANAGERSERIALIZATIONHELPER_H
