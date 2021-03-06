/**
 * @file   DepthOfField.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.02.11
 *
 * @brief  Declaration of the depth of field effect.
 */

#ifndef DEPTHOFFIELD_H
#define DEPTHOFFIELD_H

#include "main.h"

namespace cgu {

    class GPUProgram;
    class GLTexture;
    class PerspectiveCamera;
    class GLRenderTarget;
    class ScreenQuadRenderable;

    struct DOFParams
    {
        float focusZ;
        float apertureRadius;
    };

    class DepthOfField
    {
    public:
        explicit DepthOfField(const glm::ivec2 sourceSize, ApplicationBase* app);
        ~DepthOfField();

        void RenderParameterSliders();
        void ApplyEffect(const PerspectiveCamera& cam, const GLTexture* color, const GLTexture* depth, const GLTexture* targetRT);
        void Resize(const glm::uvec2& screenSize);

        void SaveParameters(std::ostream& ostr) const;
        void LoadParameters(std::istream& istr);

    private:
        static const unsigned int VERSION = 1;
        static const unsigned int RT_SIZE_FACTOR = 1;
        using FrontBackTargets = std::array<std::unique_ptr<GLTexture>, 2>;

        float CalculateFocalLength(const PerspectiveCamera& cam) const;
        float CalculateCoCRadius(const PerspectiveCamera& cam, float z) const;
        float CalculateMaxCoCRadius(const PerspectiveCamera& cam) const;

        /** Holds the render target for storing color information with circle of confusion. */
        std::unique_ptr<GLTexture> cocRT;
        /** Holds the temporary results of the blurring. */
        std::array<FrontBackTargets, 2> blurRTs;
        /** Holds the bloom parameters. */
        DOFParams params;

        /** Holds the GPU program used for glare detection. */
        std::shared_ptr<GPUProgram> cocProgram;
        /** Holds the glare program uniform ids. */
        std::vector<BindingLocation> cocUniformIds;
        /** Holds the GPU program used for blurring. */
        std::shared_ptr<GPUProgram> hBlurProgram;
        /** Holds the blur program uniform ids. */
        std::vector<BindingLocation> hBlurUniformIds;
        /** Holds the GPU program used for blurring. */
        std::shared_ptr<GPUProgram> vBlurProgram;
        /** Holds the blur program uniform ids. */
        std::vector<BindingLocation> vBlurUniformIds;
        /** Holds the GPU program used for combining the final image. */
        std::shared_ptr<GPUProgram> combineProgram;
        /** Holds the combine program uniform ids. */
        std::vector<BindingLocation> combineUniformIds;

        /** Holds the size of the source textures. */
        glm::ivec2 sourceRTSize;
    };
}

#endif // DEPTHOFFIELD_H
