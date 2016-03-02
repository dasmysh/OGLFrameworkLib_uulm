/**
 * @file   PerspectiveCamera.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.03.02
 *
 * @brief  Definition of a camera class.
 */

#ifndef PERSPECTIVECAMERA_H
#define PERSPECTIVECAMERA_H

#include "main.h"
#include <glm/gtc/quaternion.hpp>
#include "core/math/math.h"

namespace cgu {

    class ShaderBufferBindingPoints;
    class GLUniformBuffer;

    /**
     *  Represents a pinhole camera independent of input.
     */
    class PerspectiveCamera
    {
    public:
        PerspectiveCamera(float fovY, const glm::uvec2& theScreenSize, float nearZ, float farZ, const glm::vec3& camPos,
            ShaderBufferBindingPoints* uniformBindingPoints);
        PerspectiveCamera(const PerspectiveCamera&);
        PerspectiveCamera& operator=(const PerspectiveCamera&);
        PerspectiveCamera(PerspectiveCamera&&);
        PerspectiveCamera& operator=(PerspectiveCamera&&);
        virtual ~PerspectiveCamera();

        void ResetCamera(const glm::mat4& proj, const glm::mat4& view);
        void Resize(const glm::uvec2& screenSize);
        void SetView() const;
        void SetViewShadowMap() const;
        void RotateOrigin(const glm::quat& camOrientStep);
        void MoveCamera(const glm::vec3& translation);

        cguMath::Frustum<float> GetViewFrustum(const glm::mat4& modelM) const;
        float GetSignedDistance2ToUnitAABB(const glm::mat4& world) const;
        glm::vec2 CalculatePixelFootprintToUnitAABB(const glm::mat4& world) const;
        float GetSignedDistanceToUnitAABB2(const glm::mat4& world) const;

        const glm::mat4& GetViewMatrix() const { return view_; }
        const glm::mat4& GetProjMatrix() const { return perspective_; }
        const glm::vec3& GetPosition() const { return camPos_; }
        const glm::quat& GetOrientation() const { return camOrient_; }
        float GetFOV() const { return fovY_; }
        void SetFOV(float fov);
        float GetNearZ() const { return nearZ_; }
        float GetFarZ() const { return farZ_; }
        const glm::uvec2& GetScreenSize() const { return screenSize_; }

    private:
        cguMath::Frustum<float> CalcViewFrustum(const glm::mat4& mvp) const;

        /** Holds the field of view in y direction. */
        float fovY_;
        /** Holds the aspect ratio. */
        float aspectRatio_;
        /** Holds the screen size. */
        glm::uvec2 screenSize_;
        /** Holds the near z plane. */
        float nearZ_;
        /** Holds the far z plane. */
        float farZ_;
        /** Holds the perspective transform matrix. */
        glm::mat4 perspective_;
        /** Holds the current camera position. */
        glm::vec3 camPos_;
        /** Holds the current camera orientation. */
        glm::quat camOrient_;
        /** holds the current up vector of the camera. */
        glm::vec3 camUp_;
        /** Holds the cameras view matrix. */
        glm::mat4 view_;
        /** Holds the perspective projection uniform buffer. */
        std::unique_ptr<GLUniformBuffer> perspectiveUBO_;
    };
}

#endif // PERSPECTIVECAMERA_H
