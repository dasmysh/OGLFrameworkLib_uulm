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
#include <glm/gtc/matrix_transform.hpp>

namespace cgu {

    static unsigned int calcTextureMaxSize(const glm::uvec3& volumeSize)
    {
        return std::max(std::max(volumeSize.x, volumeSize.y), volumeSize.z);
    }

    static unsigned int calcMipLevels(unsigned int volumeMaxSize)
    {
        auto fNumLevels = 1.0f + glm::floor(glm::log2(static_cast<float>(volumeMaxSize)));
        return static_cast<unsigned int>(fNumLevels);
    }

    MinMaxVolume::MinMaxVolume(const std::shared_ptr<const Volume>& texData, ApplicationBase* app) :
        volumeData(texData),
        volumeTexture(nullptr),
        minMaxTexture(nullptr),
        minMaxProgram(nullptr),
        minMaxLevelsProgram(nullptr),
        volumeSize(volumeData->GetSize()),
        texMax(static_cast<float>(calcTextureMaxSize(volumeSize))),
        voxelScale(volumeData->GetScaling() * glm::vec3(volumeSize) / static_cast<float>(calcTextureMaxSize(volumeSize)))
    {
        auto numLevels = calcMipLevels(static_cast<unsigned int>(texMax));
        stepSizes.resize(numLevels, 1.0f / (2.0f * texMax));

        volumeTexture = volumeData->Load3DTexture(numLevels);

        std::string shaderDefines;
        auto minMaxDesc = volumeTexture->GetDescriptor();
        minMaxDesc.bytesPP *= 4;
        minMaxDesc.format = gl::GL_RG;
        switch (minMaxDesc.internalFormat) {
        case gl::GL_R8:
            minMaxDesc.internalFormat = gl::GL_RG8;
            shaderDefines = "AVGTEX r8,MMTEX rg8";
            break;
        case gl::GL_R16F:
            minMaxDesc.internalFormat = gl::GL_RG16F;
            shaderDefines = "AVGTEX r16f,MMTEX rg16f";
            break;
        case gl::GL_R32F:
            minMaxDesc.internalFormat = gl::GL_RG32F;
            shaderDefines = "AVGTEX r32f,MMTEX rg32f";
            break;
        default:
            throw std::runtime_error("Texture format not allowed.");
        }

        mipLevelsProgram = app->GetGPUProgramManager()->GetResource("shader/minmaxmaps/genMipLevels.cp," + shaderDefines);
        mipLevelsUniformNames = mipLevelsProgram->GetUniformLocations({ "origLevelTex", "nextLevelTex" });
        mipLevelsProgram->UseProgram();
        mipLevelsProgram->SetUniform(mipLevelsUniformNames[0], 0);
        mipLevelsProgram->SetUniform(mipLevelsUniformNames[1], 1);

        auto numGroups = glm::ivec3(glm::ceil(glm::vec3(volumeSize) / 8.0f));
        auto stepSizeFactor = 1.0f;
        for (unsigned int lvl = 1; lvl < numLevels; ++lvl) {
            numGroups = glm::ivec3(glm::ceil(glm::vec3(numGroups) * 0.5f));
            stepSizeFactor *= 2.0f;
            stepSizes[lvl] *= stepSizeFactor;

            OGL_CALL(gl::glMemoryBarrier, gl::GL_ALL_BARRIER_BITS);
            OGL_SCALL(gl::glFinish);

            volumeTexture->ActivateImage(0, lvl - 1, gl::GL_READ_ONLY);
            volumeTexture->ActivateImage(1, lvl, gl::GL_WRITE_ONLY);
            OGL_CALL(gl::glDispatchCompute, numGroups.x, numGroups.y, numGroups.z);
        }
        OGL_CALL(gl::glMemoryBarrier, gl::GL_ALL_BARRIER_BITS);
        OGL_SCALL(gl::glFinish);

        auto minMaxSize = volumeTexture->GetLevelDimensions(2);
        auto minMaxLevels = calcMipLevels(calcTextureMaxSize(minMaxSize));

        minMaxTexture = std::make_unique<GLTexture>(minMaxSize.x, minMaxSize.y, minMaxSize.z, minMaxLevels, minMaxDesc, nullptr);
        minMaxProgram = app->GetGPUProgramManager()->GetResource("shader/minmaxmaps/genMinMax.cp," + shaderDefines);
        minMaxUniformNames = minMaxProgram->GetUniformLocations({ "origTex", "minMaxTex" });
        minMaxProgram->UseProgram();
        minMaxProgram->SetUniform(minMaxUniformNames[0], 0);
        minMaxProgram->SetUniform(minMaxUniformNames[1], 1);

        numGroups = glm::ivec3(glm::ceil(glm::vec3(minMaxSize) / 8.0f));
        volumeTexture->ActivateImage(0, 0, gl::GL_READ_ONLY);
        minMaxTexture->ActivateImage(1, 0, gl::GL_WRITE_ONLY);
        OGL_CALL(gl::glDispatchCompute, numGroups.x, numGroups.y, numGroups.z);

        minMaxLevelsProgram = app->GetGPUProgramManager()->GetResource("shader/minmaxmaps/genMinMaxLevels.cp," + shaderDefines);
        minMaxLevelsUniformNames = minMaxLevelsProgram->GetUniformLocations({ "origLevelTex", "nextLevelTex" });
        minMaxLevelsProgram->UseProgram();
        minMaxLevelsProgram->SetUniform(minMaxLevelsUniformNames[0], 0);
        minMaxLevelsProgram->SetUniform(minMaxLevelsUniformNames[1], 1);

        numGroups = glm::ivec3(glm::ceil(glm::vec3(minMaxSize) / 8.0f));
        for (unsigned int lvl = 1; lvl < minMaxLevels; ++lvl) {
            numGroups = glm::ivec3(glm::ceil(glm::vec3(numGroups) * 0.5f));

            OGL_CALL(gl::glMemoryBarrier, gl::GL_ALL_BARRIER_BITS);
            OGL_SCALL(gl::glFinish);

            minMaxTexture->ActivateImage(0, lvl - 1, gl::GL_READ_ONLY);
            minMaxTexture->ActivateImage(1, lvl, gl::GL_WRITE_ONLY);
            OGL_CALL(gl::glDispatchCompute, numGroups.x, numGroups.y, numGroups.z);
        }
        OGL_CALL(gl::glMemoryBarrier, gl::GL_ALL_BARRIER_BITS);
        OGL_SCALL(gl::glFinish);
    }

    /**
     *  Destructor.
     */
    MinMaxVolume::~MinMaxVolume()
    {
    }

    glm::mat4 MinMaxVolume::GetLocalWorld(const glm::mat4& world) const
    {
        return glm::scale(glm::translate(world, -0.5f * voxelScale), voxelScale);
    }

    glm::mat4 MinMaxVolume::ReverseLocalWorld(const glm::mat4& world) const
    {
        return glm::translate(glm::scale(world, 1.0f / voxelScale), 0.5f * voxelScale);
    }
}
