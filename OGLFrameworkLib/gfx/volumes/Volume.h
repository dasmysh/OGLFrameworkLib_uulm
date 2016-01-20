/**
 * @file   Volume.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.08.14
 *
 * @brief  Contains volume resource class.
 */

#ifndef VOLUME_H
#define VOLUME_H

#include "main.h"
#include "core/Resource.h"
#include "gfx/glrenderer/GLTexture.h"

namespace cgu {

    class MinMaxVolume;

    /**
     *  @brief Volume resource.
     *
     * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
     * @date   2015.08.14
     */
    class Volume : public Resource
    {
    public:
        Volume(const std::string& texFilename, ApplicationBase* app);
        Volume(const Volume&);
        Volume& operator=(const Volume&);
        Volume(Volume&&);
        Volume& operator=(Volume&&);
        virtual ~Volume();

        void Load() override;
        void Unload() override;

        std::unique_ptr<GLTexture> Load3DTexture(unsigned int mipLevels) const;

        const glm::vec3& GetScaling() const { return cellSize; }

        std::unique_ptr<MinMaxVolume> GetMinMaxTexture() const;
        Volume* GetSpeedVolume() const;
        // Volume* GetHalfResTexture(bool denoise) const;
        // std::unique_ptr<VolumeBrickOctree> GetBrickedVolume(const glm::vec3& scale, int denoiseLevel) const;
        const TextureDescriptor& GetTextureDescriptor() const { return texDesc; }
        const glm::uvec3& GetSize() const { return volumeSize; }

        // glm::uvec3 GetBrickTextureSize(const glm::uvec3& pos, const glm::uvec3& size) const;


        /*void ReadRaw(std::vector<uint8_t>& data, const glm::uvec3& pos, const glm::uvec3& dataSize,
            const glm::uvec3& texSize) const;
        static void WriteRaw(std::vector<uint8_t>& data, std::fstream& fileStream, const glm::uvec3& pos,
            const glm::uvec3& dataSize, const glm::uvec3& volumeSize, unsigned int bytesPV);*/

    private:
        /** Holds the textures size. */
        glm::uvec3 volumeSize;
        /** Holds the size of each cell. */
        glm::vec3 cellSize;
        /** Holds the filename of the raw file. */
        std::string rawFileName;
        /** Holds a scale value for the stored data. */
        unsigned int scaleValue;
        /** Holds the dimension of the data. */
        int dataDim;
        /** Holds the texture description. */
        TextureDescriptor texDesc;

        void LoadDatFile();
        void LoadRawDataFromFile(unsigned& data_size, std::vector<char>& rawData) const;
    };
}

#endif // VOLUME_H

