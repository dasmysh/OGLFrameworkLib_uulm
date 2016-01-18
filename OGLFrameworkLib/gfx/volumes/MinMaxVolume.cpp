/**
 * @file   MinMaxVolume.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.01.15
 *
 * @brief  Implementation of a volume with average, minimum and maximum values.
 */

#include "MinMaxVolume.h"
#include "gfx/volumes/Volume.h"
#include "gfx/glrenderer/GLTexture.h"
#include "app/ApplicationBase.h"
#include <boost/assign.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace cgu {

    static unsigned int calcMipLevelSize(unsigned int baseSz, unsigned int level)
    {
        return static_cast<unsigned int>(glm::max(1.0f, glm::floor(static_cast<float>(baseSz)
            / static_cast<float>(glm::pow(2, level)))));
    }

    static unsigned int calcMipLevels(const glm::uvec3& volumeSize)
    {
        auto maxSize = std::max(std::max(volumeSize.x, volumeSize.y), volumeSize.z);
        auto fNumLevels = 1.0f + glm::floor(glm::log2(static_cast<float>(maxSize)));
        return static_cast<unsigned int>(fNumLevels);
    }

    MinMaxVolume::MinMaxVolume(const Volume* texData, ApplicationBase* app) :
        volumeData(texData),
        volumeTexture(nullptr),
        minMaxTexture(nullptr),
        minMaxProgram(nullptr),
        minMaxLevelsProgram(nullptr)
    {
        auto volumeSize = volumeData->GetSize();
        auto numLevels = calcMipLevels(volumeSize);

        volumeTexture = volumeData->Load3DTexture(numLevels);

        auto minMaxSize = glm::uvec3(calcMipLevelSize(volumeSize.x, 2),
            calcMipLevelSize(volumeSize.y, 2),
            calcMipLevelSize(volumeSize.z, 2));
        auto minMaxLevels = calcMipLevels(minMaxSize);

        std::string shaderDefines;
        auto minMaxDesc = volumeTexture->GetDescriptor();
        minMaxDesc.bytesPP *= 2;
        minMaxDesc.format = GL_RG;
        switch (minMaxDesc.internalFormat) {
        case GL_R8:
            minMaxDesc.internalFormat = GL_RG8;
            shaderDefines = "AVGTEX r8,MMTEX rg8";
            break;
        case GL_R16F:
            minMaxDesc.internalFormat = GL_RG16F;
            shaderDefines = "AVGTEX r16f,MMTEX rg16f";
            break;
        case GL_R32F:
            minMaxDesc.internalFormat = GL_RG32F;
            shaderDefines = "AVGTEX r32f,MMTEX rg32f";
            break;
        default:
            throw std::runtime_error("Texture format not allowed.");
        }
        minMaxTexture = std::make_unique<GLTexture>(minMaxSize.x, minMaxSize.y, minMaxSize.z, minMaxLevels, minMaxDesc, nullptr);
        minMaxProgram = app->GetGPUProgramManager()->GetResource("shader/minmaxmaps/genMinMax.cp," + shaderDefines);
        minMaxUniformNames = minMaxProgram->GetUniformLocations({ "origTex", "minMaxTex" });
        minMaxProgram->UseProgram();
        minMaxProgram->SetUniform(minMaxUniformNames[0], 0);
        minMaxProgram->SetUniform(minMaxUniformNames[1], 1);

        auto numGroups = glm::ivec3(glm::ceil(glm::vec3(minMaxSize) / 8.0f));
        volumeTexture->ActivateImage(0, 0, GL_READ_ONLY);
        minMaxTexture->ActivateImage(1, 0, GL_WRITE_ONLY);
        OGL_CALL(glDispatchCompute, numGroups.x, numGroups.y, numGroups.z);

        minMaxLevelsProgram = app->GetGPUProgramManager()->GetResource("shader/minmaxmaps/genMinMaxLevels.cp," + shaderDefines);
        minMaxLevelsUniformNames = minMaxProgram->GetUniformLocations({ "origLevel", "nextLevel" });
        minMaxLevelsProgram->UseProgram();
        minMaxLevelsProgram->SetUniform(minMaxLevelsUniformNames[0], 0);
        minMaxLevelsProgram->SetUniform(minMaxLevelsUniformNames[1], 1);

        for (unsigned int lvl = 1; lvl < minMaxLevels; ++lvl) {
            numGroups = glm::ivec3(glm::ceil(glm::vec3(minMaxSize) / (8.0f * static_cast<float>(lvl + 1))));

            OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
            OGL_SCALL(glFinish);

            minMaxTexture->ActivateImage(0, lvl - 1, GL_READ_ONLY);
            minMaxTexture->ActivateImage(1, lvl, GL_WRITE_ONLY);
            OGL_CALL(glDispatchCompute, numGroups.x, numGroups.y, numGroups.z);
        }
        OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
        OGL_SCALL(glFinish);
    }

    void MinMaxVolume::CalculateTexBorders(const GLTexture3D* texData, const glm::uvec3& pos,
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
    MinMaxVolume::~VolumeBrickOctree()
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
    const GLTexture3D* MinMaxVolume::GetDownscaledVolume(bool denoise) const
    {
        return volumeData->GetHalfResTexture(denoise);
    }

    /**
     *  Reloads all data from this tree node.
     */
    void MinMaxVolume::ReloadData()
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
    void MinMaxVolume::ResetData()
    {
        brickTexture.reset(nullptr);
    }

    /**
     *  Releases all the data from this tree node and its children.
     */
    void MinMaxVolume::ResetAllData()
    {
        if (hasAnyData) {
            brickTexture.reset(nullptr);
            if (children[0]) for (auto& child : children) child->ResetAllData();
            hasAnyData = false;
        }
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
