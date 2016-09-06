/**
 * @file   ArcballRotatedSceneRenderable.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.05.28
 *
 * @brief  Implementation of the arcball rotated scene object.
 */

#include "ArcballRotatedSceneRenderable.h"
#include "gfx/ArcballCamera.h"
#include <GLFW/glfw3.h>

namespace cgu {

    ArcballRotatedSceneRenderable::ArcballRotatedSceneRenderable(MeshRenderable* renderable, const glm::vec3& pos) :
        SceneRenderable(renderable, pos, glm::quat(1.0f, 0.0f, 0.0f, 0.0f)),
        rotArcball(GLFW_MOUSE_BUTTON_RIGHT)
    {

    }

    ArcballRotatedSceneRenderable::ArcballRotatedSceneRenderable(const glm::vec3& pos) :
        SceneRenderable(pos, glm::quat(1.0f, 0.0f, 0.0f, 0.0f)),
        rotArcball(GLFW_MOUSE_BUTTON_RIGHT)
    {

    }

    ArcballRotatedSceneRenderable::~ArcballRotatedSceneRenderable()
    {
    }

    bool ArcballRotatedSceneRenderable::HandleMouse(int button, int action, int mods, float, GLWindow* sender)
    {
        return rotArcball.HandleMouse(button, action, mods, sender);
    }

    void ArcballRotatedSceneRenderable::Update(const ArcballCamera& camera, float, float)
    {
        auto orient = glm::inverse(rotArcball.GetWorldRotation(camera.GetViewMatrix())) * GetOrientation();
        UpdatePositionOrientation(GetPosition(), orient);
    }
}
