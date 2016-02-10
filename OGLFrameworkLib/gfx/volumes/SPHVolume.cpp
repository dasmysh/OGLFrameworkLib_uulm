/**
 * @file   SPHVolume.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.02.09
 *
 * @brief  Implementation of a volume with spherical harmonics downsampling.
 */

#include "SPHVolume.h"
#include "gfx/volumes/Volume.h"
#include "gfx/glrenderer/GLTexture.h"
#include "app/ApplicationBase.h"
#include <glm/gtc/matrix_transform.hpp>

namespace cgu {

    static unsigned int calcMipLevelSize(unsigned int baseSz, unsigned int level)
    {
        return static_cast<unsigned int>(glm::max(1.0f, glm::floor(static_cast<float>(baseSz)
            / static_cast<float>(glm::pow(2, level)))));
    }

    static unsigned int calcTextureMaxSize(const glm::uvec3& volumeSize)
    {
        return std::max(std::max(volumeSize.x, volumeSize.y), volumeSize.z);
    }

    static unsigned int calcMipLevels(unsigned int volumeMaxSize)
    {
        auto fNumLevels = 1.0f + glm::floor(glm::log2(static_cast<float>(volumeMaxSize)));
        return static_cast<unsigned int>(fNumLevels);
    }

    SPHVolume::SPHVolume(const std::shared_ptr<const Volume>& texData, ApplicationBase* app) :
        volumeData(texData),
        volumeTexture(nullptr),
        sphProgram(nullptr),
        sphLevelsProgram(nullptr),
        volumeSize(volumeData->GetSize()),
        texMax(static_cast<float>(calcTextureMaxSize(volumeSize))),
        voxelScale(volumeData->GetScaling() * glm::vec3(volumeSize) / static_cast<float>(calcTextureMaxSize(volumeSize)))
    {
        auto numLevels = calcMipLevels(static_cast<unsigned int>(texMax));
        stepSizes.resize(numLevels, 1.0f / (2.0f * texMax));

        volumeTexture = volumeData->Load3DTexture(3);

        std::string shaderDefines;
        auto sphDesc = volumeTexture->GetDescriptor();
        sphDesc.bytesPP *= 4;
        sphDesc.format = GL_RGBA;
        switch (sphDesc.internalFormat) {
        case GL_R8:
            sphDesc.internalFormat = GL_RGBA8;
            shaderDefines = "TEX r8,SPHTEX rgba8";
            break;
        case GL_R16F:
            sphDesc.internalFormat = GL_RGBA16F;
            shaderDefines = "TEX r16f,SPHTEX rgba16f";
            break;
        case GL_R32F:
            sphDesc.internalFormat = GL_RGBA32F;
            shaderDefines = "TEX r32f,SPHTEX rgba32f";
            break;
        default:
            throw std::runtime_error("Texture format not allowed.");
        }

        /*mipLevelsProgram = app->GetGPUProgramManager()->GetResource("shader/minmaxmaps/genMipLevels.cp," + shaderDefines);
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

            OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
            OGL_SCALL(glFinish);

            volumeTexture->ActivateImage(0, lvl - 1, GL_READ_ONLY);
            volumeTexture->ActivateImage(1, lvl, GL_WRITE_ONLY);
            OGL_CALL(glDispatchCompute, numGroups.x, numGroups.y, numGroups.z);
        }
        OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
        OGL_SCALL(glFinish);*/

        auto sphSize = volumeTexture->GetLevelDimensions(2);
        auto sphLevels = calcMipLevels(calcTextureMaxSize(sphSize)) - 4;
        auto twoRootPi = 2.0f * glm::root_pi<float>();
        sphCoeffs = glm::vec2(1.0f / twoRootPi, glm::root_three<float>() / twoRootPi);

        sphTextures[0] = std::make_unique<GLTexture>(sphSize.x, sphSize.y, sphSize.z, sphLevels, sphDesc, nullptr);
        sphTextures[1] = std::make_unique<GLTexture>(sphSize.x, sphSize.y, sphSize.z, sphLevels, sphDesc, nullptr);
        sphProgram = app->GetGPUProgramManager()->GetResource("shader/sphvolumes/genSPHMap.cp," + shaderDefines);
        sphUniformNames = sphProgram->GetUniformLocations({ "origTex", "sphTex0", "sphTex1", "sphCoeffs" });
        sphProgram->UseProgram();
        sphProgram->SetUniform(sphUniformNames[0], 0);
        sphProgram->SetUniform(sphUniformNames[1], 1);
        sphProgram->SetUniform(sphUniformNames[2], 2);
        sphProgram->SetUniform(sphUniformNames[3], sphCoeffs);

        auto numGroups = glm::ivec3(glm::ceil(glm::vec3(sphSize) / 8.0f));
        volumeTexture->ActivateImage(0, 0, GL_READ_ONLY);
        for (unsigned int lvl = 0; lvl < sphLevels; ++lvl) {
            OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
            OGL_SCALL(glFinish);

            sphTextures[0]->ActivateImage(1, lvl, GL_WRITE_ONLY);
            sphTextures[1]->ActivateImage(2, lvl, GL_WRITE_ONLY);
            OGL_CALL(glDispatchCompute, numGroups.x, numGroups.y, numGroups.z);
            numGroups = glm::ivec3(glm::ceil(glm::vec3(numGroups) * 0.5f));
        }
        OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
        OGL_SCALL(glFinish);


        /*volumeTexture->ActivateImage(0, 0, GL_READ_ONLY);
        sphTextures[0]->ActivateImage(1, 0, GL_WRITE_ONLY);
        sphTextures[1]->ActivateImage(2, 0, GL_WRITE_ONLY);
        OGL_CALL(glDispatchCompute, numGroups.x, numGroups.y, numGroups.z);

        minMaxLevelsProgram = app->GetGPUProgramManager()->GetResource("shader/minmaxmaps/genMinMaxLevels.cp," + shaderDefines);
        minMaxLevelsUniformNames = minMaxLevelsProgram->GetUniformLocations({ "origLevelTex", "nextLevelTex" });
        minMaxLevelsProgram->UseProgram();
        minMaxLevelsProgram->SetUniform(minMaxLevelsUniformNames[0], 0);
        minMaxLevelsProgram->SetUniform(minMaxLevelsUniformNames[1], 1);

        numGroups = glm::ivec3(glm::ceil(glm::vec3(minMaxSize) / 8.0f));
        for (unsigned int lvl = 1; lvl < minMaxLevels; ++lvl) {
            numGroups = glm::ivec3(glm::ceil(glm::vec3(numGroups) * 0.5f));

            OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
            OGL_SCALL(glFinish);

            minMaxTexture->ActivateImage(0, lvl - 1, GL_READ_ONLY);
            minMaxTexture->ActivateImage(1, lvl, GL_WRITE_ONLY);
            OGL_CALL(glDispatchCompute, numGroups.x, numGroups.y, numGroups.z);
        }
        OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
        OGL_SCALL(glFinish);*/
    }

    /**
     *  Destructor.
     */
    SPHVolume::~SPHVolume()
    {
    }

    glm::mat4 SPHVolume::GetLocalWorld(const glm::mat4& world) const
    {
        return glm::scale(glm::translate(world, -0.5f * voxelScale), voxelScale);
    }
}
