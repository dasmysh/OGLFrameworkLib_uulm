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
    class ArcballCamera;
    class GPUProgram;
    class GLTexture;

    class ShadowMap
    {
    public:
        ShadowMap(const glm::uvec2& size, const SpotLight& light, ApplicationBase* app);
        ~ShadowMap();

        void RenderShadowGeometry(std::function<void(GLBatchRenderTarget&) > batch);
        void Resize(const glm::uvec2& smSize);
        const glm::uvec2& GetSize() const { return shadowMapSize; }
        static glm::mat4 GetViewProjectionTextureMatrix(const glm::mat4& view, const glm::mat4& projection);
        const GLTexture* GetShadowTexture() const;

    private:
        /** Holds the spot light using this shadow map. */
        const SpotLight& spotLight;
        /** Holds the size of the shadow map. */
        glm::uvec2 shadowMapSize;
        /** Holds the render target for the shadow map. */
        std::unique_ptr<GLRenderTarget> shadowMapRT;
        /** Holds the blurred shadow map texture. */
        std::unique_ptr<GLTexture> blurredShadowMap;

        /** Holds the shader used for rendering the shadow map. */
        std::shared_ptr<GPUProgram> smProgram;
        /** Holds the shader used for filtering the shadow map. */
        std::shared_ptr<GPUProgram> filterProgram;
        /** Holds the uniforms used for filtering the shadow map. */
        std::vector<BindingLocation> filterUniformIds;
    };
}

#endif // SHADOWMAP_H
