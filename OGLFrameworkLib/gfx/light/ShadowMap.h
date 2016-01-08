#pragma once
/**
 * @file   ShadowMap.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.01.08
 *
 * @brief  Declaration of the shadow map class.
 */

#ifndef SHADOWMAP_H
#define SHADOWMAP_H

#include "main.h"

namespace cgu {
    class ApplicationBase;
    class GLRenderTarget;
    class GLBatchRenderTarget;
    class SpotLight;
    class CameraView;
    class GPUProgram;

    class ShadowMap
    {
    public:
        ShadowMap(const glm::uvec2& size, const SpotLight& light, ApplicationBase* app);
        ~ShadowMap();

        void RenderShadowGeometry(std::function<void(const CameraView&, GLBatchRenderTarget&) > batch);

    private:
        /** Holds the spot light using this shadow map. */
        const SpotLight& spotLight;
        /** Holds the render target for the shadow map. */
        std::unique_ptr<GLRenderTarget> shadowMapRT;
        /** Holds the shader used for rendering shadow map. */
        GPUProgram* smProgram;
    };
}

#endif // SHADOWMAP_H
