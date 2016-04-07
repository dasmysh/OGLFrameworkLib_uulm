/**
 * @file   ArcballCamera.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.05.27
 *
 * @brief  Implementation of the free fly camera.
 */

#define GLM_SWIZZLE

#include "ArcballCamera.h"
#include <glm/gtc/quaternion.hpp>
#include <GLFW/glfw3.h>

namespace cgu {

    /**
     *  Constructor.
     *  @param theFov the cameras field of view.
     *  @param theScreenSize the cameras screen size.
     *  @param theNearZ distance to the near clipping plane.
     *  @param theFarZ distance to the far clipping plane.
     *  @param theCamPos the cameras position.
     *  @param uniformBindingPoints uniform buffer binding points for the camera used for shadow map rendering.
     */
    ArcballCamera::ArcballCamera(int button, float theFovY, const glm::uvec2& theScreenSize, float theNearZ,
        float theFarZ, const glm::vec3& theCamPos,
        ShaderBufferBindingPoints* uniformBindingPoints) :
        PerspectiveCamera(theFovY, theScreenSize, theNearZ, theFarZ, theCamPos, uniformBindingPoints),
        camArcball_(button)
    {
    }

    /**
     *  Constructor.
     *  @param theFov the cameras field of view.
     *  @param theScreenSize the cameras screen size.
     *  @param theNearZ distance to the near clipping plane.
     *  @param theFarZ distance to the far clipping plane.
     *  @param theCamPos the cameras position.
     *  @param uniformBindingPoints uniform buffer binding points for the camera used for shadow map rendering.
     */
    ArcballCamera::ArcballCamera(float theFovY, const glm::uvec2& theScreenSize, float theNearZ, float theFarZ,
        const glm::vec3& theCamPos, ShaderBufferBindingPoints* uniformBindingPoints) :
        ArcballCamera(GLFW_MOUSE_BUTTON_LEFT, theFovY, theScreenSize, theNearZ, theFarZ, theCamPos, uniformBindingPoints)
    {
    }

    /**
     *  Copy constructor.
     */
    ArcballCamera::ArcballCamera(const ArcballCamera& rhs) :
        PerspectiveCamera(rhs),
        camArcball_(rhs.camArcball_)
    {
    }

    /**
     *  Copy assignment operator.
     */
    ArcballCamera& ArcballCamera::operator=(const ArcballCamera& rhs)
    {
        if (this != &rhs) {
            ArcballCamera tmp{ rhs };
            std::swap(*this, tmp);
        }
        return *this;
    }

    /**
     *  Move constructor.
     */
    ArcballCamera::ArcballCamera(ArcballCamera&& rhs) :
        PerspectiveCamera(std::move(rhs)),
        camArcball_(std::move(rhs.camArcball_))
    {
    }

    /**
     *  Move assignment operator.
     */
    ArcballCamera& ArcballCamera::operator=(ArcballCamera&& rhs)
    {
        if (this != &rhs) {
            this->~ArcballCamera();
            PerspectiveCamera* tPCam = this;
            *tPCam = static_cast<PerspectiveCamera&&>(std::move(rhs));
            camArcball_ = std::move(rhs.camArcball_);
        }
        return *this;
    }

    /** Default destructor. */
    ArcballCamera::~ArcballCamera() = default;

    /**
     *  Updates the cameras position and directions.
     */
    void ArcballCamera::UpdateCamera()
    {
        auto camOrientStep = camArcball_.GetWorldRotation(GetViewMatrix());
        RotateOrigin(camOrientStep);
    }

    /**
     *  Handles keyboard input for camera positioning.
     *  @param vkCode the virtual key code of the key event.
     *  @param vKeyDown whether the key is down or not.
     *  @param sender the window the event came from.
     */
    bool ArcballCamera::HandleKeyboard(int key, int, int action, int, GLWindow*)
    {
        if (action == GLFW_RELEASE) return false;
        glm::vec3 translation(0.0f);
        auto handled = false;
        switch (key) {
        case GLFW_KEY_W:
            translation -= glm::vec3(0.0f, 0.0f, 0.5f);
            handled = true;
            break;
        case GLFW_KEY_A:
            translation -= glm::vec3(0.5f, 0.0f, 0.0f);
            handled = true;
            break;
        case GLFW_KEY_S:
            translation += glm::vec3(0.0f, 0.0f, 0.5f);
            handled = true;
            break;
        case GLFW_KEY_D:
            translation += glm::vec3(0.5f, 0.0f, 0.0f);
            handled = true;
            break;
        }

        MoveCamera(translation);
        return handled;
    }

    /**
     *  Handles the mouse input for camera positioning.
     *  @param buttonAction the button event.
     *  @param mouseWheelDelta the mouse wheel movement.
     *  @param sender the window the event came from.
     */
    bool ArcballCamera::HandleMouse(int button, int action, int mods, float mouseWheelDelta, GLWindow* sender)
    {
        auto handled = camArcball_.HandleMouse(button, action, mods, sender);

        if (mouseWheelDelta != 0) {
            auto fov = GetFOV() - mouseWheelDelta * 0.03f;
            fov = glm::clamp(fov, 1.0f, 80.0f);
            SetFOV(fov);
            handled = true;
        }

        return handled;
    }
}
