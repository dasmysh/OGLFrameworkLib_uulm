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

        // bool UpdateFrustum(const cgu::CameraView& camera, const glm::mat4& world, float pixelThreshold = 0.8f);
        glm::mat4 GetLocalWorld(const glm::mat4& world) const;
        float CheckLodLevel(const cgu::CameraView& camera, const glm::mat4& world) const;
        glm::vec3 GetWorldScale() const { return voxelScale * glm::vec3(noOvlpSize); };
        bool IsLoaded() const { return static_cast<bool>(brickTexture); }
        const GLTexture* GetTexture() const { return brickTexture.get(); }
        const glm::vec3& GetMinTexCoord() const { return minTexValue; }
        const glm::vec3& GetMaxTexCoord() const { return maxTexValue; }
        float GetTexMax() const { return texMax; }
        bool IsMaxLevel() const { return level == maxLevel; }

    private:
        /*VolumeBrickOctree(const glm::uvec3& pos, const glm::uvec3& size, const glm::vec3& scale,
            unsigned int lvl, GPUProgram* minMaxProg, const std::vector<BindingLocation> uniformNames, FILE* streamFile);
        VolumeBrickOctree(const GLTexture3D* texData, const glm::uvec3& posOffset, const glm::uvec3& size,
            unsigned int lvl, const glm::vec3& scale, int denoiseLevel, GPUProgram* minMaxProg,
            const std::vector<BindingLocation> uniformNames, FILE* streamFile);*/

        /*void CreateNode(const glm::uvec3& childSizeBase, int denoiseLevel, const GLTexture3D* texData);
        void CreateLeafTexture(const GLTexture3D* texData);
        void CalculateTexBorders(const GLTexture3D* texData, const glm::uvec3& pos, const glm::uvec3& size,
            unsigned int overlap);
        bool UpdateFrustumInternal(const cgu::CameraView& camera, const glm::mat4& world, float voxelsInPixel,
            float pixelThreshold = 0.8f);
        const GLTexture3D* GetDownscaledVolume(bool denoise) const;*/

        // void WriteBrickTextureToTempFile();
        void ResetData();
        void ResetAllData();
        void ReloadData();

        /** Holds the 3D texture object to load from. */
        const Volume* volumeData;

        /** Holds the texture containing the original data and MipMaps. */
        std::unique_ptr<GLTexture> volumeTexture;
        /** Holds the texture containing the min/max data. */
        std::unique_ptr<GLTexture> minMaxTexture;
        /** Holds the GPUProgram for generating the top level min max texture. */
        GPUProgram* minMaxProgram;
        /** Holds the binding locations for the program generating the top level min max texture. */
        std::vector<BindingLocation> minMaxUniformNames;
        /** Holds the GPUProgram for generating the lower of the min max texture. */
        GPUProgram* minMaxLevelsProgram;
        /** Holds the binding locations for the program generating the lower of the min max texture. */
        std::vector<BindingLocation> minMaxLevelsUniformNames;

        /** Holds the scaling of a voxel. */
        const glm::vec3 voxelScale;

        /** Holds the maximum texture dimension. */
        float texMax;

    };
}

#endif // MINMAXVOLUME_H
