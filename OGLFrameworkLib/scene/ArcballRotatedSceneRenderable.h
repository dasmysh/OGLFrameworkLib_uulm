/**
 * @file   ArcballRotatedSceneRenderable.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.05.28
 *
 * @brief  Definition of a scene object that is rotated by an arcball.
 */

#ifndef ARCBALLROTATEDSCENERENDERABLE_H
#define ARCBALLROTATEDSCENERENDERABLE_H

#include "SceneRenderable.h"
#include "core/Arcball.h"

namespace cgu {

    class PerspectiveCamera;

    class ArcballRotatedSceneRenderable : public SceneRenderable
    {
    public:
        ArcballRotatedSceneRenderable(MeshRenderable* renderable, const glm::vec3& pos);
        virtual ~ArcballRotatedSceneRenderable();

        virtual bool HandleMouse(int button, int action, int mods, float mouseWheelDelta, GLWindow* sender);
        virtual void Update(const PerspectiveCamera& camera, float time, float elapsed);
        virtual void Resize(const glm::uvec2& screenSize) = 0;

    protected:
        /** Constructor used for derivations that create their own renderables. */
        explicit ArcballRotatedSceneRenderable(const glm::vec3& pos);

    private:
        /** Holds the arcball for rotation. */
        Arcball rotArcball;
    };
}

#endif /* ARCBALLROTATEDSCENERENDERABLE_H */
