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

        std::unique_ptr<GLTexture> Load3DTexture(unsigned int mipLevels) const;

        const glm::vec3& GetScaling() const { return cellSize; }

        std::shared_ptr<Volume> GetSpeedVolume() const;
        const TextureDescriptor& GetTextureDescriptor() const { return texDesc; }
        const glm::uvec3& GetSize() const { return volumeSize; }

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
        void LoadRawDataFromFile(unsigned int& data_size, std::vector<char>& rawData) const;
    };
}

#endif // VOLUME_H

