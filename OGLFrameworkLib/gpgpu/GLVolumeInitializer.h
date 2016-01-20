/**
 * @file   GLVolumeInitializer.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.09.21
 *
 * @brief  Contains declaration of a helper class to initialize volumes with a compute shader.
 */

#ifndef GLVOLUMEINITIALIZER_H
#define GLVOLUMEINITIALIZER_H

#include "gfx/glrenderer/GLTexture.h"

namespace cgu {

    class Volume;

    namespace gpgpu {

        class GLVolumeInitializer
        {
        public:
            GLVolumeInitializer(unsigned int width, unsigned int height, unsigned int depth, const TextureDescriptor& desc);
            virtual ~GLVolumeInitializer();

            Volume* InitChecker(const std::string& filename, const glm::uvec3& checkerSize, ApplicationBase* app) const;
            Volume* InitStripes(const std::string& filename, unsigned int stripeSize, ApplicationBase* app) const;
            Volume* InitSpherical(const std::string& filename, const glm::vec3& sphereCenter, const glm::vec3& sphereScale, ApplicationBase* app) const;

        private:
            Volume* InitGeneral(const std::string& filename, GPUProgram* initProg,
                const std::vector<BindingLocation>& uniformNames, ApplicationBase* app,
                std::function<void(const glm::uvec3&, const glm::uvec3&, const TextureDescriptor&, std::fstream&)> chunkInitialize) const;
            static void ReadRaw(std::vector<uint8_t>& data, std::fstream& fileStream, const glm::uvec3& pos,
                const glm::uvec3& dataSize, const glm::uvec3& volumeSize, unsigned int bytesPV);
            static void WriteRaw(std::vector<uint8_t>& data, std::fstream& fileStream, const glm::uvec3& pos,
                const glm::uvec3& dataSize, const glm::uvec3& volumeSize, unsigned int bytesPV);

            /** Holds the volume size. */
            glm::uvec3 volSize;
            /** Holds the texture description. */
            TextureDescriptor texDesc;
        };
    }
}

#endif // GLVOLUMEINITIALIZER_H
