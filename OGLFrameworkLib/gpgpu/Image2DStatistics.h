/**
 * @file   Image2DStatistics.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.09.18
 *
 * @brief  Contains a class to create statistics of 2D images.
 */

#ifndef IMAGE2DSTATISTICS_H
#define IMAGE2DSTATISTICS_H

#include <string>
#include "main.h"

namespace cgu {

    class GLTexture;
    class GLTexture2D;
    class GPUProgram;
    class ApplicationBase;

    namespace eval {

        struct EvalStatistics
        {
            float errorMax_;
            float numErrorPixels_;
            float errorRMSAvg_;
            float errorRMSAvgAll_;
            float errorRMSMax_;
            float errorRMSMaxAll_;
            float psnrAvg_;
            float psnrAvgAll_;
            float psnrMax_;
            float psnrMaxAll_;
        };

        class Image2DStatistics
        {
        public:
            Image2DStatistics(const std::string& origImage, ApplicationBase* app);
            ~Image2DStatistics();

            EvalStatistics CreateDiffImage(const std::string& compareImage, const std::string& diffImage) const;
            /*glm::vec2 GetRMSErrorDeltaE() const;
            glm::vec3 GetRMSErrorMax() const;
            glm::vec2 GetRMSErrorAvg() const;*/

        private:
            /** Holds the texture of the original image. */
            std::unique_ptr<GLTexture2D> origTex_;
            /** Holds the texture of the difference image. */
            std::unique_ptr<GLTexture> diffTex_;
            /** Holds the texture of the statistics image. */
            std::unique_ptr<GLTexture> statsTex_;
            /** Holds the shader used for calculating a difference image. */
            std::shared_ptr<GPUProgram> diffProgram_;
            /** Holds the uniforms used for calculating a difference image. */
            std::vector<BindingLocation> diffUniformIds_;
            /** Holds the shader used for reducing the statistics image. */
            std::shared_ptr<GPUProgram> reduceProgram_;
            /** Holds the uniforms used for reducing the statistics image. */
            std::vector<BindingLocation> reduceUniformIds_;

            /** Holds the images dimensions. */
            glm::uvec3 imgDimensions_;

            /** Holds the application object. */
            ApplicationBase* application_;
        };
    }
}

#endif // IMAGE2DSTATISTICS_H
