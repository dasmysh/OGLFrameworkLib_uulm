/**
 * @file   MinMaxVolume.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.01.15
 *
 * @brief  Defines a volume with average, minimum and maximum values.
 */

#ifndef MINMAXVOLUME_H
#define MINMAXVOLUME_H

#include "main.h"
#include "gfx/glrenderer/GLTexture.h"
#include <boost/filesystem.hpp>

namespace cgu {

    class Volume;
    class GPUProgram;
    class CameraView;
    class ApplicationBase;

    class MinMaxVolume
    {
    public:
        MinMaxVolume(const Volume* texData, ApplicationBase* app);
        ~MinMaxVolume();

        glm::mat4 GetLocalWorld(const glm::mat4& world) const;
        const GLTexture* GetVolumeTexture() const { return volumeTexture.get(); }
        const GLTexture* GetMinMaxTexture() const { return minMaxTexture.get(); }
        float GetTexMax() const { return texMax; }

    private:
        /** Holds the 3D texture object to load from. */
        const Volume* volumeData;

        /** Holds the texture containing the original data and MipMaps. */
        std::unique_ptr<GLTexture> volumeTexture;
        /** Holds the texture containing the min/max data. */
        std::unique_ptr<GLTexture> minMaxTexture;
        /** Holds the GPUProgram for generating the lower mip map levels. */
        GPUProgram* mipLevelsProgram;
        /** Holds the binding locations for the program generating the lower mip map levels. */
        std::vector<BindingLocation> mipLevelsUniformNames;
        /** Holds the GPUProgram for generating the top level min max texture. */
        GPUProgram* minMaxProgram;
        /** Holds the binding locations for the program generating the top level min max texture. */
        std::vector<BindingLocation> minMaxUniformNames;
        /** Holds the GPUProgram for generating the lower min max levels. */
        GPUProgram* minMaxLevelsProgram;
        /** Holds the binding locations for the program generating the lower min max levels. */
        std::vector<BindingLocation> minMaxLevelsUniformNames;

        /** Holds the volumes size. */
        glm::uvec3 volumeSize;
        /** Holds the maximum texture dimension. */
        float texMax;

        /** Holds the scaling of a voxel. */
        const glm::vec3 voxelScale;

    };
}

#endif // MINMAXVOLUME_H
