/**
 * @file   CameraView.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.05.27
 *
 * @brief  Definition of a free fly camera view.
 */

#ifndef CAMERAVIEW_H
#define CAMERAVIEW_H

#include "core/Arcball.h"
#include "main.h"
#include <glm/gtc/quaternion.hpp>
#include "core/math/math.h"

namespace cgu {

    class ShaderBufferBindingPoints;
    class GLUniformBuffer;

    /**
    * @brief  Represents a free fly camera responding to input and to be used for rendering.
    *
    * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
    * @date   2015.05.27
    */
    class CameraView
    {
    public:
        CameraView(unsigned int theButtonDownFlag, unsigned int theButtonFlag, float fovY,
            const glm::uvec2& theScreenSize, float nearZ, float farZ, const glm::vec3& camPos,
            ShaderBufferBindingPoints* uniformBindingPoints);
        CameraView(float fovY, const glm::uvec2& theScreenSize, float nearZ, float farZ, const glm::vec3& camPos,
            ShaderBufferBindingPoints* uniformBindingPoints);
        CameraView(const CameraView&);
        CameraView& operator=(const CameraView&);
        CameraView(CameraView&&);
        CameraView& operator=(CameraView&&);
        virtual ~CameraView();

        void ResetCamera(const glm::mat4& proj, const glm::mat4& view);
        void Resize(const glm::uvec2& screenSize);
        bool HandleKeyboard(unsigned int vkCode, bool bKeyDown, BaseGLWindow* sender);
        bool HandleMouse(unsigned int buttonAction, float mouseWheelDelta, BaseGLWindow* sender);
        void SetView() const;
        void SetViewShadowMap() const;
        void UpdateCamera();
        const glm::mat4& GetViewMatrix() const { return view; }
        glm::mat4 GetProjMatrix() const { return perspective; }
        cguMath::Frustum<float> GetViewFrustum(const glm::mat4& modelM) const;
        const glm::vec3& GetPosition() const { return camPos; }
        float GetSignedDistance2ToUnitAABB(const glm::mat4& world) const;
        glm::vec2 CalculatePixelFootprintToUnitAABB(const glm::mat4& world) const;
        float GetSignedDistanceToUnitAABB2(const glm::mat4& world) const;
        float GetFOV() const { return fovY; }
        void SetFOV(float fov);
        float GetNearZ() const { return nearZ; }
        float GetFarZ() const { return farZ; }
        const glm::uvec2& GetScreenSize() const { return screenSize; }

    private:
        cguMath::Frustum<float> CalcViewFrustum(const glm::mat4& mvp) const;

        /** Holds the field of view in y direction. */
        float fovY;
        /** Holds the aspect ratio. */
        float aspectRatio;
        /** Holds the screen size. */
        glm::uvec2 screenSize;
        /** Holds the near z plane. */
        float nearZ;
        /** Holds the far z plane. */
        float farZ;
        /** Holds the perspective transform matrix. */
        glm::mat4 perspective;
        /** Holds the current camera position. */
        glm::vec3 camPos;
        /** Holds the current camera orientation. */
        glm::quat camOrient;
        /** holds the current up vector of the camera. */
        glm::vec3 camUp;
        /** Holds the cameras view matrix. */
        glm::mat4 view;
        /** Holds the arcball used for camera rotation. */
        Arcball camArcball;
        /** Holds the perspective projection uniform buffer. */
        std::unique_ptr<GLUniformBuffer> perspectiveUBO;
    };
}

#endif /* CAMERAVIEW_H */
