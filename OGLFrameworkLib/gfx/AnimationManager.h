/**
 * @file   AnimationManager.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.08.25
 *
 * @brief  Declaration of the animation manager.
 */

#ifndef ANIMATIONMANAGER_H
#define ANIMATIONMANAGER_H

#include "main.h"
#include "WaypointAnimation.h"

namespace cgu {

    class AnimationManager
    {
    public:
        explicit AnimationManager(const std::string& dir);
        ~AnimationManager();

        unsigned int AddAnimation(const std::string& name);
        WaypointAnimation& operator[] (unsigned int id) { return animations_[id]; }
        const WaypointAnimation& operator[] (unsigned int id) const { return animations_[id]; }
        WaypointAnimation& operator[] (const std::string& name) { return *animationsByName_[name].second; }
        const WaypointAnimation& operator[] (const std::string& name) const { return *animationsByName_.at(name).second; }
        WaypointAnimation& GetCurrent() { return animations_[currentAnimation_]; }
        const WaypointAnimation& GetCurrent() const { return animations_[currentAnimation_]; }

        void ShowAnimationMenu(const std::string& name);
        void LoadAnimation(const std::string& filename, int set);
        void SaveAnimation(const std::string& filename, int set);
        void LoadAll(const std::string& filename);
        void SaveAll(const std::string& filename);

    private:
        /** Holds all animations. */
        std::vector<WaypointAnimation> animations_;
        /** Holds animations by name. */
        std::map<std::string, std::pair<unsigned int, WaypointAnimation*>> animationsByName_;
        /** Holds the currently edited animation id. */
        unsigned int currentAnimation_ = 0;
        /** Holds the save/load directory. */
        std::string directory_;

    };
}

#endif // ANIMATIONMANAGER_H
