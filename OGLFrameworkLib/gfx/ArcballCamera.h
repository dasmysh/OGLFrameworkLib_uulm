/**
 * @file   ArcballCamera.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.05.27
 *
 * @brief  Definition of a free fly camera view.
 */

#ifndef ARCBALLCAMERA_H
#define ARCBALLCAMERA_H

#include "core/Arcball.h"
#include "PerspectiveCamera.h"

namespace cgu {

    class ShaderBufferBindingPoints;
    class GLUniformBuffer;

    /**
    * @brief  Represents a arc-ball camera responding to input and to be used for rendering.
    *
    * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
    * @date   2015.05.27
    */
    class ArcballCamera : public PerspectiveCamera
    {
    public:
        ArcballCamera(int button, float fovY, const glm::uvec2& theScreenSize, float nearZ, float farZ,
            const glm::vec3& camPos, ShaderBufferBindingPoints* uniformBindingPoints);
        ArcballCamera(float fovY, const glm::uvec2& theScreenSize, float nearZ, float farZ, const glm::vec3& camPos,
            ShaderBufferBindingPoints* uniformBindingPoints);
        ArcballCamera(const ArcballCamera&);
        ArcballCamera& operator=(const ArcballCamera&);
        ArcballCamera(ArcballCamera&&);
        ArcballCamera& operator=(ArcballCamera&&);
        virtual ~ArcballCamera();

        bool HandleKeyboard(int key, int scancode, int action, int mods, GLWindow* sender);
        bool HandleMouse(int button, int action, int mods, float mouseWheelDelta, GLWindow* sender);
        void UpdateCamera();

    private:
        /** Holds the arc-ball used for camera rotation. */
        Arcball camArcball_;
    };
}

#endif /* ARCBALLCAMERA_H */
