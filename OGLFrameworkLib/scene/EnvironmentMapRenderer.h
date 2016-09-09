/**
 * @file   EnvironmentMapRenderer.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.09.08
 *
 * @brief  Declaration of a renderer for environment maps.
 */

#ifndef ENVIRONMENTMAPRENDERER_H
#define ENVIRONMENTMAPRENDERER_H

#include "main.h"

namespace cgu {\

    class GLTexture;
    class PerspectiveCamera;
    class GPUProgram;
    class ScreenQuadRenderable;

    class EnvironmentMapRenderer
    {
    public:
        explicit EnvironmentMapRenderer(ApplicationBase* app);
        ~EnvironmentMapRenderer();

        void Draw(const PerspectiveCamera& camera, const GLTexture& tex);
        void Draw(const glm::vec3& camPos, const glm::mat4& view, const glm::mat4& proj, const GLTexture& tex);

    private:
        /** Holds the shader. */
        std::shared_ptr<GPUProgram> envMapProgram_;
        /** Holds the uniform bindings. */
        std::vector<BindingLocation> envMapUniformIds_;
        /** Holds the screen quad renderable. */
        ScreenQuadRenderable& screenQuad_;
    };
}


#endif // ENVIRONMENTMAPRENDERER_H
