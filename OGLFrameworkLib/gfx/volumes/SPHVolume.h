/**
 * @file   SPHVolume.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.02.09
 *
 * @brief  Defines a volume with spherical harmonics downsampling.
 */

#ifndef SPHVOLUME_H
#define SPHVOLUME_H

#include "main.h"
#include "gfx/glrenderer/GLTexture.h"

namespace cgu {

    class Volume;
    class GPUProgram;
    class CameraView;
    class ApplicationBase;

    class SPHVolume
    {
    public:
        SPHVolume(const std::shared_ptr<const Volume>& texData, ApplicationBase* app);
        ~SPHVolume();

        glm::mat4 GetLocalWorld(const glm::mat4& world) const;
        const GLTexture* GetVolumeTexture() const { return volumeTexture.get(); }
        const GLTexture* GetSPHTexture(int i) const { return sphTextures[i].get(); }
        float GetTexMax() const { return texMax; }
        float GetStepSize(unsigned int mipLevel) const { return stepSizes[mipLevel]; };
        const glm::vec2& GetSPHCoeffs() const { return sphCoeffs; };

    private:
        static const unsigned int NUM_SHELLS = 2;
        /** Holds the 3D texture object to load from. */
        std::shared_ptr<const Volume> volumeData;

        /** Holds the texture containing the original data and MipMaps. */
        std::unique_ptr<GLTexture> volumeTexture;
        /** Holds the texture containing the sph data. */
        std::array<std::unique_ptr<GLTexture>, NUM_SHELLS> sphTextures;
        /** Holds the GPUProgram for generating the lower mip map levels. */
        // std::shared_ptr<GPUProgram> mipLevelsProgram;
        /** Holds the binding locations for the program generating the lower mip map levels. */
        std::vector<BindingLocation> mipLevelsUniformNames;
        /** Holds the GPUProgram for generating the top level min max texture. */
        std::shared_ptr<GPUProgram> sphProgram;
        /** Holds the binding locations for the program generating the top level min max texture. */
        std::vector<BindingLocation> sphUniformNames;
        /** Holds the GPUProgram for generating the lower min max levels. */
        std::shared_ptr<GPUProgram> sphLevelsProgram;
        /** Holds the binding locations for the program generating the lower min max levels. */
        std::vector<BindingLocation> sphLevelsUniformNames;

        /** Holds the volumes size. */
        glm::uvec3 volumeSize;
        /** Holds the maximum texture dimension. */
        float texMax;
        /** Holds the step sizes for the mip levels. */
        std::vector<float> stepSizes;
        /** Holds the spherical harmonics coefficients. */
        glm::vec2 sphCoeffs;

        /** Holds the scaling of a voxel. */
        const glm::vec3 voxelScale;

    };
}

#endif // SPHVOLUME_H
