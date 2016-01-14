/**
 * @file   VolumeBrickOctree.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.08.17
 *
 * @brief  Implementation of the layered octree.
 */

#include "VolumeBrickOctree.h"
#include "gfx/glrenderer/GLTexture3D.h"
#include "gfx/glrenderer/GLTexture.h"
#include "app/ApplicationBase.h"
#include <boost/assign.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace cgu {

    namespace cguOctreeMath {
        inline unsigned int calculateOverlapPixels(unsigned int val) {
            return (val / VolumeBrickOctree::MAX_SIZE) << 1;
        }
    }

    VolumeBrickOctree::VolumeBrickOctree(const glm::uvec3& pos, const glm::uvec3& size, const glm::vec3& scale,
        unsigned int lvl, GPUProgram* minMaxProg, const std::vector<BindingLocation> uniformNames, FILE* strFile) :
        volumeData(nullptr),
        posOffset(pos),
        origSize(size),
        voxelScale(scale),
        level(lvl),
        hasAnyData(true),
        noOvlpPos(posOffset),
        noOvlpSize(origSize),
        minTexValue(0.0f),
        maxTexValue(1.0f),
        maxLevel(0),
        texMax(static_cast<float>(MAX_SIZE)),
        streamFile(strFile),
        dataPos(0),
        dataSize(0),
        brickTextureDesc(4, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE),
        minMaxMapProgram(minMaxProg),
        minMaxMapUniformNames(uniformNames)
    {
    }

    /**
     *  Constructor.
     *  @param texData the 3D texture resource to create the tree from.
     *  @param pos the position in the texture.
     *  @param size the size of this part of the tree (in voxels).
     *  @param scale the scale of a voxel in this tree.
     *  @param denoiseLevel the level of denoising to do.
     *  @param minMaxProg the program used to create Min-/Max-Maps.
     *  @param uniformNames the uniform names of the minMaxProg.
     */
    VolumeBrickOctree::VolumeBrickOctree(const GLTexture3D* texData, const glm::uvec3& pos, const glm::uvec3& size,
        const glm::vec3& scale, int denoiseLevel, GPUProgram* minMaxProg, const std::vector<BindingLocation> uniformNames) :
        VolumeBrickOctree(pos, size, scale, 0, minMaxProg, uniformNames, nullptr)
    {
        tempPath = boost::filesystem::unique_path("volbricks-%%%%%%%-tmp");
        boost::filesystem::create_directory(tempPath);
        auto tmpPathStr = tempPath.string() + std::string("/streamfile.tmp");
        streamFile = std::fopen(tmpPathStr.c_str(), "wb+");

        if (origSize.x > MAX_SIZE || origSize.y > MAX_SIZE || origSize.z > MAX_SIZE) {
            auto ovlp = cguOctreeMath::calculateOverlapPixels(glm::max(origSize.x, glm::max(origSize.y, origSize.z)));
            glm::uvec3 sizeOverlap{ ovlp };
            auto sizeWithOverlap = origSize + sizeOverlap;
            glm::uvec3 sizePowerOfTwo{ cguMath::roundupPow2(origSize.x), cguMath::roundupPow2(origSize.y),
                cguMath::roundupPow2(origSize.z) };
            if (sizeWithOverlap.x > sizePowerOfTwo.x) {
                sizeWithOverlap.x = cguMath::roundupPow2(sizeWithOverlap.x);
                sizeOverlap <<= 1;
            } else sizeWithOverlap.x = sizePowerOfTwo.x;
            if (sizeWithOverlap.y > sizePowerOfTwo.y) {
                sizeWithOverlap.y = cguMath::roundupPow2(sizeWithOverlap.y);
                sizeOverlap <<= 1;
            } else sizeWithOverlap.y = sizePowerOfTwo.y;
            if (sizeWithOverlap.z > sizePowerOfTwo.z) {
                sizeWithOverlap.z = cguMath::roundupPow2(sizeWithOverlap.z);
                sizeOverlap <<= 1;
            } else sizeWithOverlap.z = sizePowerOfTwo.z;
            glm::uvec3 childSizeBase{ sizeWithOverlap.x >> 1, sizeWithOverlap.y >> 1, sizeWithOverlap.z >> 1 };

            CreateNode(childSizeBase, denoiseLevel, texData);
        } else {
            CreateLeafTexture(texData);
        }
    }

    VolumeBrickOctree::VolumeBrickOctree(const GLTexture3D* texData, const glm::uvec3& pos,
        const glm::uvec3& size, unsigned int lvl, const glm::vec3& scale, int denoiseLevel, GPUProgram* minMaxProg,
        const std::vector<BindingLocation> uniformNames, FILE* strFile) :
        VolumeBrickOctree(pos, size, scale, lvl, minMaxProg, uniformNames, strFile)
    {
        if (origSize.x * origSize.y * origSize.z == 0) {
            dataSize = 0;
            hasAnyData = false;
            return;
        }

        if (origSize.x > MAX_SIZE || origSize.y > MAX_SIZE || origSize.z > MAX_SIZE) {
            glm::uvec3 sizePowerOfTwo{ cguMath::roundupPow2(origSize.x), cguMath::roundupPow2(origSize.y),
                cguMath::roundupPow2(origSize.z) };
            glm::uvec3 childSizeBase{ sizePowerOfTwo.x >> 1, sizePowerOfTwo.y >> 1, sizePowerOfTwo.z >> 1 };

            CreateNode(childSizeBase, denoiseLevel, texData);

        } else {
            CreateLeafTexture(texData);
        }
    }
    
    void VolumeBrickOctree::CreateNode(const glm::uvec3& childSizeBase, int denoiseLevel, const GLTexture3D* texData)
    {
        glm::uvec3 posOffsets[8];
        posOffsets[0] = glm::uvec3(0, 0, 0);
        posOffsets[1] = glm::uvec3(0, 0, 1);
        posOffsets[2] = glm::uvec3(0, 1, 0);
        posOffsets[3] = glm::uvec3(0, 1, 1);
        posOffsets[4] = glm::uvec3(1, 0, 0);
        posOffsets[5] = glm::uvec3(1, 0, 1);
        posOffsets[6] = glm::uvec3(1, 1, 0);
        posOffsets[7] = glm::uvec3(1, 1, 1);

        auto ovlp = cguOctreeMath::calculateOverlapPixels(glm::max(childSizeBase.x, glm::max(childSizeBase.y, childSizeBase.z)));

        for (unsigned int i = 0; i < 8; ++i) {
            auto childPosOffset = posOffsets[i] * (childSizeBase - glm::uvec3(ovlp));
            auto childPos = posOffset + childPosOffset;
            auto childSize = glm::uvec3(glm::max(glm::ivec3(0),
                glm::ivec3(glm::min(origSize, childPosOffset + childSizeBase)) - glm::ivec3(childPosOffset)));

            if (childSizeBase.x == ovlp) {
                if (posOffsets[i].x == 0) childSize.x += ovlp;
                else if (posOffsets[i].x == 1) childSize.x = 0;
            }
            if (childSizeBase.y == ovlp) {
                if (posOffsets[i].y == 0) childSize.y += ovlp;
                else if (posOffsets[i].y == 1) childSize.y = 0;
            }
            if (childSizeBase.z == ovlp) {
                if (posOffsets[i].z == 0) childSize.z += ovlp;
                else if (posOffsets[i].z == 1) childSize.z = 0;
            }

            children[i].reset(new VolumeBrickOctree(texData, childPos, childSize, level + 1, voxelScale, denoiseLevel,
                minMaxMapProgram, minMaxMapUniformNames, streamFile));// , app));

            maxLevel = glm::max(maxLevel, children[i]->maxLevel);
        }

        if (static_cast<int>(maxLevel - level) <= denoiseLevel) {
            volumeData = children[0]->GetDownscaledVolume(true);
        } else {
            volumeData = children[0]->GetDownscaledVolume(false);
        }
        auto lowResPosOffset = posOffset >> (maxLevel - level);
        auto lowResSize = origSize >> (maxLevel - level);
        texSize = volumeData->GetBrickTextureSize(lowResPosOffset, lowResSize);
        CalculateTexBorders(volumeData, lowResPosOffset, lowResSize, ovlp);

        brickTextureDesc = volumeData->GetTextureDescriptor();
        std::vector<uint8_t> data;
        volumeData->ReadRaw(data, lowResPosOffset, lowResSize, texSize);
        brickTexture.reset(new GLTexture(texSize.x, texSize.y, texSize.z, MAX_MIPLEVELS, brickTextureDesc, data.data()));
        WriteBrickTextureToTempFile();

        for (auto& child : children) child->ResetAllData();
    }

    void VolumeBrickOctree::CreateLeafTexture(const GLTexture3D* texData)
    {
        if (origSize.x * origSize.y * origSize.z == 0) {
            dataSize = 0;
            hasAnyData = false;
            return;
        }

        maxLevel = level;
        volumeData = texData;
        texSize = volumeData->GetBrickTextureSize(posOffset, origSize);

        CalculateTexBorders(volumeData, posOffset, origSize, 1);

        brickTextureDesc = volumeData->GetTextureDescriptor();
        std::vector<uint8_t> data;
        volumeData->ReadRaw(data, posOffset, origSize, texSize);
        brickTexture.reset(new GLTexture(texSize.x, texSize.y, texSize.z, MAX_MIPLEVELS, brickTextureDesc, data.data()));
        WriteBrickTextureToTempFile();
    }

    void VolumeBrickOctree::CalculateTexBorders(const GLTexture3D* texData, const glm::uvec3& pos,
        const glm::uvec3&, unsigned int overlap)
    {
        auto pixelOffset = 1.0f;
        if (pos.x != 0) {
            minTexValue.x = pixelOffset / static_cast<float>(texSize.x);
            noOvlpPos.x += overlap;
        }
        if (pos.y != 0) {
            minTexValue.y = pixelOffset / static_cast<float>(texSize.y);
            noOvlpPos.y += overlap;
        }
        if (pos.z != 0) {
            minTexValue.z = pixelOffset / static_cast<float>(texSize.z);
            noOvlpPos.z += overlap;
        }

        if (pos.x + texSize.x <= texData->GetSize().x) {
            maxTexValue.x = (static_cast<float>(texSize.x) - pixelOffset) / static_cast<float>(texSize.x);
            noOvlpSize.x -= overlap;
        }
        if (pos.y + texSize.y <= texData->GetSize().y) {
            maxTexValue.y = (static_cast<float>(texSize.y) - pixelOffset) / static_cast<float>(texSize.y);
            noOvlpSize.y -= overlap;
        }
        if (pos.z + texSize.z <= texData->GetSize().z) {
            maxTexValue.z = (static_cast<float>(texSize.z) - pixelOffset) / static_cast<float>(texSize.z);
            noOvlpSize.z -= overlap;
        }

        texMax = static_cast<float>(std::max(std::max(texSize.x, texSize.y), texSize.z));
    }

    /**
     *  Destructor.
     */
    VolumeBrickOctree::~VolumeBrickOctree()
    {
        if (level == 0 && streamFile) {
            std::fclose(streamFile);
            streamFile = nullptr;
            boost::filesystem::remove_all(tempPath);
        }
    }

    /**
     *  Creates a downscaled version of the texture used for this node.
     *  @param denoise whether the volume data should be de-noised.
     *  @return the downscaled texture.
     */
    const GLTexture3D* VolumeBrickOctree::GetDownscaledVolume(bool denoise) const
    {
        return volumeData->GetHalfResTexture(denoise);
    }

    /**
     *  Reloads all data from this tree node.
     */
    void VolumeBrickOctree::ReloadData()
    {
        if (dataSize == 0) return;
        std::fseek(streamFile, dataPos, SEEK_SET);
        std::vector<uint8_t> data;
        data.resize(dataSize);
        std::fread(data.data(), sizeof(uint8_t), dataSize, streamFile);

        brickTexture.reset(new GLTexture(texSize.x, texSize.y, texSize.z, MAX_MIPLEVELS, brickTextureDesc, nullptr));
        brickTexture->UploadData(data);
        brickTexture->GenerateMinMaxMaps(minMaxMapProgram, minMaxMapUniformNames);
        brickTexture->SampleLinear();
        brickTexture->SampleWrapClamp();
    }

    /**
     *  Releases all the data from this tree node.
     */
    void VolumeBrickOctree::ResetData()
    {
        brickTexture.reset(nullptr);
    }

    /**
     *  Releases all the data from this tree node and its children.
     */
    void VolumeBrickOctree::ResetAllData()
    {
        if (hasAnyData) {
            brickTexture.reset(nullptr);
            if (children[0]) for (auto& child : children) child->ResetAllData();
            hasAnyData = false;
        }
    }

    /**
     *  Writes the brick texture to a temporary file.
     */
    void VolumeBrickOctree::WriteBrickTextureToTempFile()
    {
        if (texSize.x * texSize.y * texSize.z == 0) {
            dataSize = 0;
            hasAnyData = false;
            return;
        }
        std::vector<uint8_t> data;
        brickTexture->DownloadData(data);

        dataSize = static_cast<unsigned int>(data.size());
        std::fseek(streamFile, 0, SEEK_END);
        dataPos = std::ftell(streamFile);
        std::fwrite(data.data(), sizeof(uint8_t), dataSize, streamFile);
        ReloadData();
    }

    /**
     *  Updates the loading state of the tree depending on the view frustum (in object space).
     *  @param camera the current camera to calculate the frustum for.
     *  @param world the world matrix of the parent node.
     *  @param pixelThreshold the size of a voxel in pixels until a higher resolution is used.
     *  @return whether there are any children with loaded data in the sub-tree.
     */
    bool VolumeBrickOctree::UpdateFrustum(const cgu::CameraView& camera, const glm::mat4& world, float pixelThreshold)
    {
        if (dataSize == 0) return false;
        auto localWorld = GetLocalWorld(world);
        auto voxelsInPixel = CheckLodLevel(camera, localWorld);

        auto isCorrectLod = false;
        if (pixelThreshold > 0.0f) isCorrectLod = voxelsInPixel < pixelThreshold;

        cguMath::AABB3<float> box{ { { glm::vec3(0.0f), glm::vec3(1.0f) } } };

        if (!cguMath::AABBInFrustumTest(camera.GetViewFrustum(localWorld), box)) ResetAllData();
        else if (!children[0]) {
            if (!IsLoaded()) { ReloadData(); hasAnyData = true; }
        } else {
            if (isCorrectLod) {
                if (IsLoaded()) for (auto& child : children) child->ResetAllData();
                else { ReloadData(); hasAnyData = true; }
            } else {
                if (IsLoaded()) {
                    ResetData();
                }
                hasAnyData = false;
                for (auto& child : children) hasAnyData = child->UpdateFrustumInternal(camera, world,
                    voxelsInPixel / 2.0f, pixelThreshold) || hasAnyData;
            }
        }
        return hasAnyData;
    }

    void VolumeBrickOctree::GetRenderedBricksList(const cgu::CameraView& camera, const glm::mat4& world,
        std::vector<std::pair<const VolumeBrickOctree*, float>>& result) const
    {
        if (!hasAnyData || dataSize == 0) return;
        else if (IsLoaded()) {
            auto childWorld = GetLocalWorld(world);
            result.push_back(std::make_pair(this, camera.GetSignedDistance2ToUnitAABB(childWorld)));
        } else {
            for (auto& child : children) child->GetRenderedBricksList(camera, world, result);
        }
    }

    glm::mat4 VolumeBrickOctree::GetLocalWorld(const glm::mat4& world) const
    {
        auto scaleVoxelMat = glm::scale(glm::mat4(), glm::vec3(texSize << (maxLevel - level)) * (maxTexValue - minTexValue));
        auto scaleToWorldMat = glm::scale(glm::mat4(), voxelScale);
        auto translateOffsetMat = glm::translate(glm::mat4(), glm::vec3(noOvlpPos));// +(minTexValue * glm::vec3(texSize)));
        return world * scaleToWorldMat * translateOffsetMat * scaleVoxelMat;
    }

    float VolumeBrickOctree::CheckLodLevel(const cgu::CameraView& camera, const glm::mat4& world) const
    {
        auto pixelFP = camera.CalculatePixelFootprintToUnitAABB(world) / texMax;
        return glm::max(pixelFP.x, pixelFP.y);
    }

    /**
     *  Updates the loading state of the tree depending on the view frustum (in object space).
     *  @param camera the current camera to calculate the frustum for.
     *  @param world the world matrix of the parent node.
     *  @param voxelsInPixel number of voxels in a pixel.
     *  @param pixelThreshold the size of a voxel in pixels until a higher resolution is used.
     *  @return whether there are any children with loaded data in the sub-tree.
     */
    bool VolumeBrickOctree::UpdateFrustumInternal(const cgu::CameraView& camera, const glm::mat4& world,
        float voxelsInPixel, float pixelThreshold)
    {
        if (dataSize == 0) return false;
        auto localWorld = GetLocalWorld(world);

        auto isCorrectLod = false;
        if (pixelThreshold > 0.0f) isCorrectLod = voxelsInPixel < pixelThreshold;

        cguMath::AABB3<float> box{ { { glm::vec3(0.0f), glm::vec3(1.0f) } } };

        /*if (isCorrectLod) {
            auto a = 0;
        }*/

        if (!cguMath::AABBInFrustumTest(camera.GetViewFrustum(localWorld), box)) ResetAllData();
        else if (!children[0]) {
            if (!IsLoaded()) { ReloadData(); hasAnyData = true; }
        } else {
            if (isCorrectLod) {
                if (IsLoaded()) for (auto& child : children) child->ResetAllData();
                else { ReloadData(); hasAnyData = true; }
            } else {
                if (IsLoaded()) {
                    ResetData();
                }
                hasAnyData = false;
                for (auto& child : children) hasAnyData = child->UpdateFrustumInternal(camera, world,
                    voxelsInPixel / 2.0f, pixelThreshold) || hasAnyData;
            }
        }
        return hasAnyData;
    }
}
