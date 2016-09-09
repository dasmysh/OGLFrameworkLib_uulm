/**
 * @file   ScreenCaptureHelper.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.09.06
 *
 * @brief  Declaration of a helper class for screen captures.
 */

#ifndef SCREENCAPTUREHELPER_H
#define SCREENCAPTUREHELPER_H

#include "main.h"

namespace cgu {

    class PerspectiveCamera;
    class GLRenderTarget;

    class ScreenCaptureHelper
    {
    public:
        explicit ScreenCaptureHelper(const std::string& directory, const glm::uvec2& size);
        ~ScreenCaptureHelper();

        void RenderScreenShot(const std::string& name, const PerspectiveCamera& camera, std::function<void(const PerspectiveCamera&, GLRenderTarget&)> drawFn);
        void RenderScreenShot(const std::string& name, unsigned int techniqueId, const std::string& techniqueName, const PerspectiveCamera& camera, std::function<void(unsigned int technique, const PerspectiveCamera&, GLRenderTarget&)> drawFn);
        void SetupVideo(const std::vector<std::string>& techniqueNames);
        void RenderVideo(const std::string& name, PerspectiveCamera& camera, float duration, std::function<void(unsigned technique, const PerspectiveCamera&, GLRenderTarget&)> drawFn, std::function<void(PerspectiveCamera&, float, float)> updateFn);
        void WriteStatistics();

    private:
        /** Holds the render target for screen shots. */
        std::unique_ptr<cgu::GLRenderTarget> scrShotTarget_;
        /** Holds the directory the screen shots are saved to. */
        std::string directory_;
        /** Holds the fps for each screen shot. */
        std::map<std::string, std::pair<float, float>> frameTimes_;
        /** Holds the technique names for video. */
        std::vector<std::string> techniqueNames_;
    };
}

#endif // SCREENCAPTUREHELPER_H
