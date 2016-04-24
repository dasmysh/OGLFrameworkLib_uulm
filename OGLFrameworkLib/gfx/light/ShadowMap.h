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
        ShadowMap(std::unique_ptr<GLRenderTarget>&& shadowMapRT, const SpotLight& light, const std::shared_ptr<GPUProgram>& smProgram, const std::shared_ptr<GPUProgram>& filterProgram, ApplicationBase* app);
        ShadowMap(const glm::uvec2& size, const SpotLight& light, ApplicationBase* app);
        ~ShadowMap();

        void RenderShadowGeometry(const glm::vec4& clearColor, std::function<void(GLBatchRenderTarget&) > batch);
        void BlurShadowMap();
        void Resize(const glm::uvec2& smSize);
        const glm::uvec2& GetSize() const { return shadowMapSize_; }
        static glm::mat4 GetViewProjectionTextureMatrix(const glm::mat4& view, const glm::mat4& projection);
        // const GLTexture* GetShadowTexture() const;
        const GLRenderTarget* GetShadowTarget() const { return shadowMapRT_.get(); }
        std::shared_ptr<GPUProgram> GetShadowMappingProgram() const { return smProgram_; }
        std::shared_ptr<GPUProgram> GetFilteringProgram() const { return filterProgram_; }

    private:
        void CreateBlurredTargets();

        /** Holds the spot light using this shadow map. */
        const SpotLight& spotLight_;
        /** Holds the size of the shadow map. */
        glm::uvec2 shadowMapSize_;
        /** Holds the render target for the shadow map. */
        std::unique_ptr<GLRenderTarget> shadowMapRT_;
        /** Holds the blurred shadow map texture. */
        std::vector<std::unique_ptr<GLTexture>> blurredShadowMap_;

        /** Holds the shader used for rendering the shadow map. */
        std::shared_ptr<GPUProgram> smProgram_;
        /** Holds the shader used for filtering the shadow map. */
        std::shared_ptr<GPUProgram> filterProgram_;
        /** Holds the uniforms used for filtering the shadow map. */
        std::vector<BindingLocation> filterUniformIds_;
    };
}

#endif // SHADOWMAP_H
