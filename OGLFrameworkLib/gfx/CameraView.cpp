/**
 * @file   CameraView.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.05.27
 *
 * @brief  Implementation of the free fly camera.
 */

#define GLM_SWIZZLE

#include "CameraView.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/quaternion.hpp>
#include "glrenderer/GLUniformBuffer.h"

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
    CameraView::CameraView(unsigned int theButtonDownFlag, unsigned int theButtonFlag, float theFovY,
        const glm::uvec2& theScreenSize, float theNearZ, float theFarZ, const glm::vec3& theCamPos,
        ShaderBufferBindingPoints* uniformBindingPoints) :
    fovY(theFovY),
    aspectRatio(static_cast<float>(theScreenSize.x) / static_cast<float>(theScreenSize.y)),
    screenSize(theScreenSize),
    nearZ(theNearZ),
    farZ(theFarZ),
    camPos(theCamPos),
    camOrient(1.0f, 0.0f, 0.0f, 0.0f),
    camUp(0.0f, 1.0f, 0.0f),
    camArcball(theButtonDownFlag, theButtonFlag),
    perspectiveUBO(uniformBindingPoints == nullptr ? nullptr : std::make_unique<GLUniformBuffer>(perspectiveProjectionUBBName,
        static_cast<unsigned int>(sizeof(PerspectiveTransformBuffer)), uniformBindingPoints))
    {
        Resize(screenSize);
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
    CameraView::CameraView(float theFovY, const glm::uvec2& theScreenSize, float theNearZ, float theFarZ,
        const glm::vec3& theCamPos, ShaderBufferBindingPoints* uniformBindingPoints) :
        CameraView(RI_MOUSE_LEFT_BUTTON_DOWN, MB_LEFT, theFovY, theScreenSize, theNearZ, theFarZ, theCamPos, uniformBindingPoints)
    {
    }

    /**
     *  Copy constructor.
     */
    CameraView::CameraView(const CameraView& rhs) :
        fovY(rhs.fovY),
        aspectRatio(rhs.aspectRatio),
        screenSize(rhs.screenSize),
        nearZ(rhs.nearZ),
        farZ(rhs.farZ),
        perspective(rhs.perspective),
        camPos(rhs.camPos),
        camOrient(rhs.camOrient),
        camUp(rhs.camUp),
        view(rhs.view),
        camArcball(rhs.camArcball),
        perspectiveUBO(new GLUniformBuffer(*rhs.perspectiveUBO))
    {
    }

    /**
     *  Copy assignment operator.
     */
    CameraView& CameraView::operator=(const CameraView& rhs)
    {
        if (this != &rhs) {
            CameraView tmp{ rhs };
            std::swap(*this, tmp);
        }
        return *this;
    }

    /**
     *  Move constructor.
     */
    CameraView::CameraView(CameraView&& rhs) :
        fovY(std::move(rhs.fovY)),
        aspectRatio(std::move(rhs.aspectRatio)),
        screenSize(std::move(rhs.screenSize)),
        nearZ(std::move(rhs.nearZ)),
        farZ(std::move(rhs.farZ)),
        perspective(std::move(rhs.perspective)),
        camPos(std::move(rhs.camPos)),
        camOrient(std::move(rhs.camOrient)),
        camUp(std::move(rhs.camUp)),
        view(std::move(rhs.view)),
        camArcball(std::move(rhs.camArcball)),
        perspectiveUBO(std::move(rhs.perspectiveUBO))
    {
    }

    /**
     *  Move assignment operator.
     */
    CameraView& CameraView::operator=(CameraView&& rhs)
    {
        if (this != &rhs) {
            this->~CameraView();
            fovY = rhs.fovY;
            aspectRatio = rhs.aspectRatio;
            screenSize = rhs.screenSize;
            nearZ = rhs.nearZ;
            farZ = rhs.farZ;
            perspective = rhs.perspective;
            camPos = rhs.camPos;
            camOrient = rhs.camOrient;
            camUp = rhs.camUp;
            view = rhs.view;
            camArcball = std::move(rhs.camArcball);
            perspectiveUBO = std::move(rhs.perspectiveUBO);
        }
        return *this;
    }

    /** Default destructor. */
    CameraView::~CameraView() = default;

    void CameraView::Resize(const glm::uvec2& theScreenSize)
    {
        screenSize = theScreenSize;
        aspectRatio = static_cast<float>(theScreenSize.x) / static_cast<float>(theScreenSize.y);
        perspective = glm::perspective(glm::radians(fovY), aspectRatio, nearZ, farZ);
        view = glm::lookAt(camPos, glm::vec3(0.0f), camUp);
    }

    /**
     *  Updates the cameras position and directions.
     */
    void CameraView::UpdateCamera()
    {
        auto camOrientStep = camArcball.GetWorldRotation(view);
        glm::mat3 matOrientStep{ glm::mat3_cast(camOrientStep) };
        camOrient = camOrientStep * camOrient;
        glm::mat3 matOrient{ glm::mat3_cast(camOrient) };
        
        camUp = matOrient[1];
        camPos = matOrientStep * camPos;

        view = glm::lookAt(camPos, glm::vec3(0.0f), camUp);
    }

    cguMath::Frustum<float> CameraView::SetView(const glm::mat4& modelM) const
    {
        PerspectiveTransformBuffer perspectiveBuffer;
        perspectiveBuffer.mat_m = modelM;
        perspectiveBuffer.mat_mvp = perspective * view * modelM;
        perspectiveBuffer.mat_normal = glm::mat4(glm::mat3(modelM));

        if (perspectiveUBO) {
            perspectiveUBO->UploadData(0, sizeof(PerspectiveTransformBuffer), &perspectiveBuffer);
            perspectiveUBO->BindBuffer();
        }

        return std::move(CalcViewFrustum(perspectiveBuffer.mat_mvp));
    }

    void CameraView::SetViewShadowMap(const glm::mat4& modelM) const
    {
        /** see http://www.mvps.org/directx/articles/linear_z/linearz.htm */
        auto projectionLinear = perspective;
        // auto Q = perspective[2][2];
        /*auto N = -perspective[3][2] / ( -perspective[2][2] + 1.0f);
        auto F = -perspective[3][2] / ( -perspective[2][2] - 1.0f);
        projectionLinear[2][2] /= F;
        projectionLinear[3][2] /= F;*/
        auto mvp = projectionLinear * view * modelM;

        if (perspectiveUBO) {
            perspectiveUBO->UploadData(sizeof(glm::mat4), sizeof(glm::mat4), &mvp);
            perspectiveUBO->BindBuffer();
        }
    }

    cguMath::Frustum<float> CameraView::GetViewFrustum(const glm::mat4& modelM) const
    {
        auto mvp = perspective * view * modelM;
        return std::move(CalcViewFrustum(mvp));
    }

    cguMath::Frustum<float> CameraView::CalcViewFrustum(const glm::mat4& m) const
    {
        cguMath::Frustum<float> f;
        f.left() = glm::row(m, 3) + glm::row(m, 0);
        f.rght() = glm::row(m, 3) - glm::row(m, 0);
        f.bttm() = glm::row(m, 3) + glm::row(m, 1);
        f.topp() = glm::row(m, 3) - glm::row(m, 1);
        f.nrpl() = glm::row(m, 3) + glm::row(m, 2);
        f.farp() = glm::row(m, 3) - glm::row(m, 2);

        /*f.left() = glm::vec4{ m[0][3] + m[0][0], m[1][3] + m[1][0], m[2][3] + m[2][0], m[3][3] + m[3][0] };
        f.rght() = glm::vec4{ m[0][3] - m[0][0], m[1][3] - m[1][0], m[2][3] - m[2][0], m[3][3] - m[3][0] };
        f.bttm() = glm::vec4{ m[0][3] + m[0][1], m[1][3] + m[1][1], m[2][3] + m[2][1], m[3][3] + m[3][1] };
        f.topp() = glm::vec4{ m[0][3] - m[0][1], m[1][3] - m[1][1], m[2][3] - m[2][1], m[3][3] - m[3][1] };
        f.nrpl() = glm::vec4{ m[0][3] + m[0][2], m[1][3] + m[1][2], m[2][3] + m[2][2], m[3][3] + m[3][2] };
        f.farp() = glm::vec4{ m[0][3] - m[0][2], m[1][3] - m[1][2], m[2][3] - m[2][2], m[3][3] - m[3][2] };*/
        f.left() /= glm::length(glm::vec3(f.left()));
        f.rght() /= glm::length(glm::vec3(f.rght()));
        f.bttm() /= glm::length(glm::vec3(f.bttm()));
        f.topp() /= glm::length(glm::vec3(f.topp()));
        f.nrpl() /= glm::length(glm::vec3(f.nrpl()));
        f.farp() /= glm::length(glm::vec3(f.farp()));
        return std::move(f);
    }

    float CameraView::GetSignedDistance2ToUnitAABB(const glm::mat4& world) const
    {
        auto localCamPos = glm::vec3(glm::inverse(world) * glm::vec4(camPos, 1.0f));
        auto clampedCamPos = glm::vec3(world * glm::vec4(glm::clamp(localCamPos, glm::vec3(0.0f), glm::vec3(1.0f)), 1.0f));
        return glm::dot(clampedCamPos - camPos, clampedCamPos - camPos);
    }

    glm::vec2 CameraView::CalculatePixelFootprintToUnitAABB(const glm::mat4& world) const
    {
        auto mvp = perspective * view * world;
        glm::vec4 pmin, pmax;
        pmin = pmax = mvp[3];
        for (unsigned int i = 0; i < 4; ++i) {
            for (unsigned int j = 0; j < 4; ++j) {
                if (mvp[j][i] < 0.0f) pmin[i] += mvp[j][i];
                else pmax[i] += mvp[j][i];
            }
        }
        pmin /= pmin.w;
        pmax /= pmax.w;

        cguMath::AABB2<float> ssAABB{ { { glm::vec2(pmin), glm::vec2(pmax) } } };
        ssAABB.minmax[0] = (ssAABB.minmax[0] + glm::vec2(1.0f)) * 0.5f * glm::vec2(screenSize);
        ssAABB.minmax[1] = (ssAABB.minmax[1] + glm::vec2(1.0f)) * 0.5f * glm::vec2(screenSize);

        return std::move(ssAABB.minmax[1] - ssAABB.minmax[0]);
    }

    float CameraView::GetSignedDistanceToUnitAABB2(const glm::mat4& world) const
    {
        auto localCamPos = glm::vec3(glm::inverse(world) * glm::vec4(camPos, 1.0f));
        auto clampedCamPos = glm::vec3(world * glm::vec4(glm::clamp(localCamPos, glm::vec3(0.0f), glm::vec3(1.0f)), 1.0f));
        return glm::dot(clampedCamPos - camPos, clampedCamPos - camPos);
    }

    /**
     *  Handles keyboard input for camera positioning.
     *  @param vkCode the virtual key code of the key event.
     *  @param vKeyDown whether the key is down or not.
     *  @param sender the window the event came from.
     */
    bool CameraView::HandleKeyboard(unsigned int vkCode, bool, BaseGLWindow*)
    {
        auto handled = false;
        switch (vkCode) {
        case 'W':
            camPos -= glm::vec3(0.0f, 0.0f, 0.5f);
            handled = true;
            break;
        case 'A':
            camPos -= glm::vec3(0.5f, 0.0f, 0.0f);
            handled = true;
            break;
        case 'S':
            camPos += glm::vec3(0.0f, 0.0f, 0.5f);
            handled = true;
            break;
        case 'D':
            camPos += glm::vec3(0.5f, 0.0f, 0.0f);
            handled = true;
            break;
        case VK_SPACE:
            camPos += glm::vec3(0.0f, 0.5f, 0.0f);
            handled = true;
            break;
        case VK_CONTROL:
        case VK_LCONTROL:
            camPos -= glm::vec3(0.0f, 0.5f, 0.0f);
            handled = true;
            break;
        }

        view = glm::lookAt(camPos, glm::vec3(0.0f), camUp);
        return handled;
    }

    /**
     *  Handles the mouse input for camera positioning.
     *  @param buttonAction the button event.
     *  @param mouseWheelDelta the mouse wheel movement.
     *  @param sender the window the event came from.
     */
    bool CameraView::HandleMouse(unsigned int buttonAction, float mouseWheelDelta, BaseGLWindow* sender)
    {
        auto handled = camArcball.HandleMouse(buttonAction, sender);

        if (mouseWheelDelta != 0) {
            fovY -= mouseWheelDelta * 0.03f;
            fovY = glm::clamp(fovY, 1.0f, 80.0f);
            perspective = glm::perspective(glm::radians(fovY), aspectRatio, nearZ, farZ);
            handled = true;
        }

        return handled;
    }
}
