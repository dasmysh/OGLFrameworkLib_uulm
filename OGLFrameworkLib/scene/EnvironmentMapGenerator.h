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
    class GLTexture;

    class EnvironmentMapGenerator
    {
    public:
        EnvironmentMapGenerator(unsigned int size, float nearZ, float farZ,
            const TextureDescriptor& texDesc, ApplicationBase* app);
        EnvironmentMapGenerator(const EnvironmentMapGenerator&);
        EnvironmentMapGenerator& operator=(const EnvironmentMapGenerator&);
        EnvironmentMapGenerator(EnvironmentMapGenerator&&);
        EnvironmentMapGenerator& operator=(EnvironmentMapGenerator&&);
        virtual ~EnvironmentMapGenerator();

        void Resize(unsigned int size);
        void DrawEnvMap(const glm::vec3& position, std::function<void(GLBatchRenderTarget&) > batch);
        std::unique_ptr<GLTexture> GenerateIrradianceMap(unsigned int irrMipLevel = 3) const;
        void UpdateIrradianceMap(const GPUProgram* irrProgram, const std::vector<BindingLocation>& irrUniformIds, const GLTexture* irrMap, unsigned int irrMipLevel = 3) const;

    private:
        /** Holds the frame-buffer for the cube map. */
        GLRenderTarget cubeMapRT_;
        /** Holds the spherical environment map. */
        std::unique_ptr<GLTexture> sphEnvMap_;
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

        /** Holds the GPU program used to create a spherical map. */
        std::shared_ptr<GPUProgram> sphProgram_;
        /** Holds the spherical program uniform ids. */
        std::vector<BindingLocation> sphUniformIds_;
    };
}


#endif // ENVIRONMENTMAPGENERATOR_H

#pragma once

