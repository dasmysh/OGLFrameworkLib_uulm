/**
 * @file   BloomEffect.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.12.22
 *
 * @brief  Declaration of the bloom effect.
 */

#ifndef BLOOMEFFECT_H
#define BLOOMEFFECT_H

#include "main.h"

namespace cgu {

    class GPUProgram;
    class GLTexture;

    struct BloomParams
    {
        float exposure;
        float bloomThreshold;
        float bloomWidth;
        float defocus;
        float bloomIntensity;
    };

    class BloomEffect
    {
    public:
        explicit BloomEffect(const glm::ivec2 sourceSize, ApplicationBase* app);
        ~BloomEffect();

        void RenderParameterSliders();
        void ApplyEffect(GLTexture* sourceRT, GLTexture* targetRT);
        void Resize(const glm::uvec2& screenSize);

        void SetExposure(float exposure) { params.exposure = exposure; }
        float GetExposure() const { return params.exposure; }

    private:
        using BlurPassTargets = std::array<std::unique_ptr<GLTexture>, 2>;
        static const unsigned int NUM_PASSES = 6;

        /** Holds the render target for HDR rendering. */
        std::unique_ptr<GLTexture> glaresRT;
        /** Holds the temporary results of the blurring. */
        std::array<BlurPassTargets, NUM_PASSES> blurRTs;
        /** Holds the bloom parameters. */
        BloomParams params;

        /** Holds the GPU program used for glare detection. */
        std::shared_ptr<GPUProgram> glareDetectProgram;
        /** Holds the glare program uniform ids. */
        std::vector<BindingLocation> glareUniformIds;
        /** Holds the GPU program used for blurring. */
        std::shared_ptr<GPUProgram> blurProgram;
        /** Holds the blur program uniform ids. */
        std::vector<BindingLocation> blurUniformIds;
        /** Holds the GPU program used for combining the final image. */
        std::shared_ptr<GPUProgram> combineProgram;
        /** Holds the combine program uniform ids. */
        std::vector<BindingLocation> combineUniformIds;
        /** Holds the number of compute groups. */
        glm::ivec2 sourceRTSize;
        /** Holds the texture unit ids to bind the blur textures to. */
        std::vector<int> blurTextureUnitIds;
    };
}

#endif // BLOOMEFFECT_H
