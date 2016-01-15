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

        bool UpdateFrustum(const cgu::CameraView& camera, const glm::mat4& world, float pixelThreshold = 0.8f);
        void GetRenderedBricksList(const cgu::CameraView& camera, const glm::mat4& world,
            std::vector<std::pair<const VolumeBrickOctree*, float>> &result) const;
        glm::mat4 GetLocalWorld(const glm::mat4& world) const;
        float CheckLodLevel(const cgu::CameraView& camera, const glm::mat4& world) const;
        glm::vec3 GetWorldScale() const { return voxelScale * glm::vec3(noOvlpSize); };
        bool IsLoaded() const { return static_cast<bool>(brickTexture); }
        const GLTexture* GetTexture() const { return brickTexture.get(); }
        const glm::vec3& GetMinTexCoord() const { return minTexValue; }
        const glm::vec3& GetMaxTexCoord() const { return maxTexValue; }
        float GetTexMax() const { return texMax; }
        bool IsMaxLevel() const { return level == maxLevel; }


        /** Holds the maximum resolution per brick. */
        static const unsigned int MAX_SIZE = 512;
        /** Holds the maximum number of MipMap levels to use. */
        static const unsigned int MAX_MIPLEVELS = 6;

    private:
        VolumeBrickOctree(const glm::uvec3& pos, const glm::uvec3& size, const glm::vec3& scale,
            unsigned int lvl, GPUProgram* minMaxProg, const std::vector<BindingLocation> uniformNames, FILE* streamFile);
        VolumeBrickOctree(const GLTexture3D* texData, const glm::uvec3& posOffset, const glm::uvec3& size,
            unsigned int lvl, const glm::vec3& scale, int denoiseLevel, GPUProgram* minMaxProg,
            const std::vector<BindingLocation> uniformNames, FILE* streamFile);

        void CreateNode(const glm::uvec3& childSizeBase, int denoiseLevel, const GLTexture3D* texData);
        void CreateLeafTexture(const GLTexture3D* texData);
        void CalculateTexBorders(const GLTexture3D* texData, const glm::uvec3& pos, const glm::uvec3& size,
            unsigned int overlap);
        bool UpdateFrustumInternal(const cgu::CameraView& camera, const glm::mat4& world, float voxelsInPixel,
            float pixelThreshold = 0.8f);
        const GLTexture3D* GetDownscaledVolume(bool denoise) const;

        void WriteBrickTextureToTempFile();
        void ResetData();
        void ResetAllData();
        void ReloadData();

        /** Holds the 3D texture object to load from. */
        const GLTexture3D* volumeData;
        /** Holds the position offset of this node. */
        const glm::uvec3 posOffset;
        /** Holds the original size of the volume (in this node). */
        const glm::uvec3 origSize;
        /** Holds the scaling of a voxel. */
        const glm::vec3 voxelScale;
        /** Holds the current tree level. */
        unsigned int level;
        /** Holds the actual texture size of this node. */
        glm::uvec3 texSize;
        /** Holds whether this node or any of its children has data loaded (i.e. is not culled). */
        bool hasAnyData;
        /** Holds the position of the volume without overlap. */
        glm::uvec3 noOvlpPos;
        /** Holds the size of the volume without overlap. */
        glm::uvec3 noOvlpSize;
        /** Holds the minimum value in texture space (excluding overlap). */
        glm::vec3 minTexValue;
        /** Holds the maximum value in texture space (excluding overlap).*/
        glm::vec3 maxTexValue;
        /** Holds the maximum level of the current sub-tree. */
        unsigned int maxLevel;
        /** Holds the maximum texture dimension. */
        float texMax;

        /** Holds the children in this layer. */
        std::array<std::unique_ptr<VolumeBrickOctree>, 8> children;

        /** Holds the temporary file with data for streaming. */
        std::FILE* streamFile;
        /** Holds the position in the stream file. */
        unsigned int dataPos;
        /** Holds the size of the data in the file. */
        unsigned int dataSize;
        /** Holds the texture descriptor. */
        TextureDescriptor brickTextureDesc;
        /** Holds the texture of this brick. */
        std::unique_ptr<GLTexture> brickTexture;
        /** Holds the GPUProgram for generating Min/Max MipMaps. */
        GPUProgram* minMaxMapProgram;
        /** Holds the binding locations for the Min/Max MipMaps. */
        std::vector<BindingLocation> minMaxMapUniformNames;

        /** Holds the temporary path for the stream file. */
        boost::filesystem::path tempPath;
    };
}

#endif // MINMAXVOLUME_H
