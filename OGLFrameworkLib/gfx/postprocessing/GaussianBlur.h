/**
 * @file   GaussianBlur.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.05.05
 *
 * @brief  Declaration of a Gaussian blur filter processor.
 */

#ifndef GAUSSIANBLUR_H
#define GAUSSIANBLUR_H

#include "main.h"

namespace cgu {

    class GLTexture;
    class ApplicationBase;
    class GPUProgram;

    class GaussianBlur
    {
    public:
        explicit GaussianBlur(const GLTexture* source, const std::string& texFormat, const std::string& blurColorType, const std::string& blurSwizzle, ApplicationBase* app);
        ~GaussianBlur();

        void ApplyBlur(float width = 1.0f);
        void Resize();

    private:
        /** Holds the texture to blur. */
        const GLTexture* source_;
        /** Holds the temp target. */
        std::unique_ptr<GLTexture> tmp_;
        /** Holds the texture size. */
        glm::uvec2 size_;
        /** Holds the GPU program used for glare detection. */
        std::shared_ptr<GPUProgram> gaussianProgram_;
        /** Holds the glare program uniform ids. */
        std::vector<BindingLocation> gaussianUniformIds_;
    };
}

#endif // GAUSSIANBLUR_H
