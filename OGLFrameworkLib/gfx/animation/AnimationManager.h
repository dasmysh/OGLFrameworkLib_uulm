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
#include "RotationAnimation.h"
#include "OrbitAnimation.h"

namespace cgu {

    template<class T>
    class TAnimationManager
    {
    public:
        explicit TAnimationManager(const std::string& dir, const PerspectiveCamera* camera);
        ~TAnimationManager();

        template<typename... Args>
        unsigned int AddAnimation(const std::string& name, Args&&... ctorArgs) {
            animations_.emplace_back(std::forward<Args>(ctorArgs)...);
            names_.emplace_back(name);
            return AddLastAnimation(name);
        }
        T& operator[] (unsigned int id) { return animations_[id]; }
        const T& operator[] (unsigned int id) const { return animations_[id]; }
        T& operator[] (const std::string& name) { return *animationsByName_[name].second; }
        const T& operator[] (const std::string& name) const { return *animationsByName_.at(name).second; }
        T& GetCurrent() { return animations_[currentAnimation_]; }
        const T& GetCurrent() const { return animations_[currentAnimation_]; }
        int GetCurrentId() const { return currentAnimation_; }
        typename T::Editor& GetEditor() { return editor_; }

        void StartAnimation();
        bool DoAnimationStep(float elapsedTime);

        // void ShowAnimationMenu(const std::string& name);
        void ShowAnimationMenu(const std::string& name, bool showMenu, bool showEdit);

        void UnselectCurrent() { currentAnimation_ = -1; editor_.SetCurrentEdited(nullptr); }

        void LoadAnimation(const std::string& filename, int set);
        void SaveAnimation(const std::string& filename, int set);
        void LoadAll(const std::string& filename);
        void SaveAll(const std::string& filename);
        void LoadAllFromStream(std::istream& stream);
        void SaveAllFromStream(std::ostream& stream);

    private:
        unsigned int AddLastAnimation(const std::string& name);

        /** Holds all animations. */
        std::vector<T> animations_;
        /** Holds all names. */
        std::vector<std::string> names_;
        /** Holds animations by name. */
        std::map<std::string, std::pair<unsigned int, T*>> animationsByName_;
        /** Holds the currently edited animation id. */
        int currentAnimation_ = -1;
        /** Holds the save/load directory. */
        std::string directory_;
        /** Holds the editor for this type. */
        typename T::Editor editor_;
    };

    using WPAnimationManager = TAnimationManager<WaypointAnimation>;
    using RotationAnimationManager = TAnimationManager<RotationAnimation>;
    using OrbitAnimationManager = TAnimationManager<OrbitAnimation>;

    enum class AnimationType
    {
        WAYPOINT,
        ROTATION,
        ORBIT
    };

    class AnimationManager
    {
    public:
        explicit AnimationManager(const std::string& dir, const PerspectiveCamera* camera);
        ~AnimationManager();

        void StartAnimation();
        bool DoAnimationStep(float elapsedTime);

        void ShowAnimationMenu(const std::string& name, bool showMenu);
        void LoadAnimations(const std::string& fileName);
        void SaveAnimations(const std::string& fileName);

        const WPAnimationManager& GetWaypoints() const { return wpAnimations_; }
        WPAnimationManager& GetWaypoints() { return wpAnimations_; }
        const RotationAnimationManager& GetRotations() const { return rotAnimations_; }
        RotationAnimationManager& GetRotations() { return rotAnimations_; }
        const OrbitAnimationManager& GetOrbits() const { return orbAnimations_; }
        OrbitAnimationManager& GetOrbits() { return orbAnimations_; }

        const WaypointAnimation& GetWaypoint(unsigned int id) const { return wpAnimations_[id]; }
        WaypointAnimation& GetWaypoint(unsigned int id) { return wpAnimations_[id]; }
        const RotationAnimation& GetRotation(unsigned int id) const { return rotAnimations_[id]; }
        RotationAnimation& GetRotation(unsigned int id) { return rotAnimations_[id]; }
        const OrbitAnimation& GetOrbit(unsigned int id) const { return orbAnimations_[id]; }
        OrbitAnimation& GetOrbit(unsigned int id) { return orbAnimations_[id]; }

        const WaypointAnimation& GetCurrentWaypoint() const { return wpAnimations_.GetCurrent(); }
        WaypointAnimation& GetCurrentWaypoint() { return wpAnimations_.GetCurrent(); }
        const RotationAnimation& GetCurrentRotation() const { return rotAnimations_.GetCurrent(); }
        RotationAnimation& GetCurrentRotation() { return rotAnimations_.GetCurrent(); }
        const OrbitAnimation& GetCurrentOrbit() const { return orbAnimations_.GetCurrent(); }
        OrbitAnimation& GetCurrentOrbit() { return orbAnimations_.GetCurrent(); }

        AnimationType GetCurrentType() const { return currentAnimation_; }

    private:
        /** Holds the waypoint animations. */
        TAnimationManager<WaypointAnimation> wpAnimations_;
        /** Holds the rotation animations. */
        TAnimationManager<RotationAnimation> rotAnimations_;
        /** Holds the orbit animations. */
        TAnimationManager<OrbitAnimation> orbAnimations_;
        /** Holds the currently edited animation manager. */
        AnimationType currentAnimation_ = AnimationType::WAYPOINT;
        /** Holds the save/load directory. */
        std::string directory_;
    };
}

#endif // ANIMATIONMANAGER_H
