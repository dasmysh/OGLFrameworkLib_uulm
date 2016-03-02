/**
 * @file   EnvironmentMapGenerator.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.03.02
 *
 * @brief  Definition of a generator class for environment maps from scenes.
 */

#ifndef ENVIRONMENTMAPGENERATOR_H
#define ENVIRONMENTMAPGENERATOR_H

#include <gfx/glrenderer/GLRenderTarget.h>

namespace cgu {

    class GLUniformBuffer;

    class EnvironmentMapGenerator final
    {
    public:
        EnvironmentMapGenerator(unsigned int size, float nearZ, float farZ,
            const TextureDescriptor& texDesc, ShaderBufferBindingPoints* uniformBindingPoints);
        EnvironmentMapGenerator(const EnvironmentMapGenerator&);
        EnvironmentMapGenerator& operator=(const EnvironmentMapGenerator&);
        EnvironmentMapGenerator(EnvironmentMapGenerator&&);
        EnvironmentMapGenerator& operator=(EnvironmentMapGenerator&&);
        ~EnvironmentMapGenerator();

        void Resize(unsigned int size);
        void DrawToCubeMap(const glm::vec3& position, std::function<void(GLBatchRenderTarget&) > batch);

    private:
        /** Holds the frame-buffer for the cube map. */
        GLRenderTarget cubeMapRT_;
        /** The perspective matrix for the camera. */
        glm::mat4 perspective_;
        /** The view matrices directions. */
        std::vector<glm::vec3> dir_;
        /** The view matrices up vectors. */
        std::vector<glm::vec3> up_;
        /** The view matrices right vectors. */
        std::vector<glm::vec3> right_;
        /** Holds the perspective projection uniform buffer. */
        std::unique_ptr<GLUniformBuffer> perspectiveUBO_;
    };
}


#endif // ENVIRONMENTMAPGENERATOR_H

#pragma once

