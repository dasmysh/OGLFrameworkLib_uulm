/**
 * @file   BloomEffect.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.12.22
 *
 * @brief  Implementation of the bloom effect.
 */

#include "BloomEffect.h"
#include "app/ApplicationBase.h"
#include <imgui.h>

namespace cgu {

    BloomEffect::BloomEffect(const glm::ivec2 sourceSize, ApplicationBase* app) :
        glareDetectProgram(app->GetGPUProgramManager()->GetResource("shader/tm/glareDetect.cp")),
        glareUniformIds(glareDetectProgram->GetUniformLocations({ "sourceTex", "targetTex", "exposure", "bloomThreshold" })),
        blurProgram(app->GetGPUProgramManager()->GetResource("shader/tm/blurBloom.cp")),
        blurUniformIds(blurProgram->GetUniformLocations({ "sourceTex", "targetTex", "dir", "bloomWidth" })),
        combineProgram(nullptr),
        combineUniformIds(),
        sourceRTSize(sourceSize)
    {
        params.bloomThreshold = 0.63f;
        params.bloomWidth = 1.0f;
        params.defocus = 0.2f;
        params.bloomIntensity = 1.0f;
        params.exposure = 2.0f;

        std::stringstream passesToString;
        passesToString << NUM_PASSES;
        combineProgram = app->GetGPUProgramManager()->GetResource("shader/tm/combineBloom.cp,NUM_PASSES " + passesToString.str());
        combineUniformIds = combineProgram->GetUniformLocations({ "sourceTex", "targetTex", "blurTex", "defocus", "bloomIntensity" });

        Resize(sourceSize);
    }

    BloomEffect::~BloomEffect() = default;

    void BloomEffect::RenderParameterSliders()
    {
        if (ImGui::TreeNode("Bloom Parameters"))
        {
            ImGui::InputFloat("Bloom Threshold", &params.bloomThreshold, 0.01f);
            ImGui::InputFloat("Bloom Width", &params.bloomWidth, 0.1f);
            ImGui::InputFloat("Bloom Defocus", &params.defocus, 0.01f);
            ImGui::InputFloat("Bloom Intensity", &params.bloomIntensity, 0.1f);
            ImGui::TreePop();
        }
    }

    void BloomEffect::ApplyEffect(GLTexture* sourceRT, GLTexture* targetRT)
    {
        const glm::vec2 groupSize{ 32.0f, 16.0f };

        auto targetSize = glm::vec2(sourceRTSize) / 2.0f;
        auto numGroups = glm::ivec2(glm::ceil(targetSize / groupSize));
        glareDetectProgram->UseProgram();
        glareDetectProgram->SetUniform(glareUniformIds[0], 0);
        glareDetectProgram->SetUniform(glareUniformIds[1], 0);
        glareDetectProgram->SetUniform(glareUniformIds[2], params.exposure);
        glareDetectProgram->SetUniform(glareUniformIds[3], params.bloomThreshold);
        sourceRT->ActivateTexture(gl::GL_TEXTURE0);
        glaresRT->ActivateImage(0, 0, gl::GL_WRITE_ONLY);
        OGL_CALL(gl::glDispatchCompute, numGroups.x, numGroups.y, 1);
        OGL_CALL(gl::glMemoryBarrier, gl::GL_ALL_BARRIER_BITS);
        OGL_SCALL(gl::glFinish);

        auto base = 1.0f;
        auto current = glaresRT.get();
        for (auto& blurPassRTs : blurRTs) {
            targetSize = glm::vec2(sourceRTSize) / base;
            numGroups = glm::ivec2(glm::ceil(targetSize / groupSize));

            blurProgram->UseProgram();
            blurProgram->SetUniform(blurUniformIds[0], 0);
            blurProgram->SetUniform(blurUniformIds[1], 0);
            blurProgram->SetUniform(blurUniformIds[2], glm::vec2(1.0f, 0.0f));
            blurProgram->SetUniform(blurUniformIds[3], params.bloomWidth);
            current->ActivateTexture(gl::GL_TEXTURE0);
            blurPassRTs[0]->ActivateImage(0, 0, gl::GL_WRITE_ONLY);
            OGL_CALL(gl::glDispatchCompute, numGroups.x, numGroups.y, 1);
            OGL_CALL(gl::glMemoryBarrier, gl::GL_ALL_BARRIER_BITS);
            OGL_SCALL(gl::glFinish);

            blurProgram->SetUniform(blurUniformIds[2], glm::vec2(0.0f, 1.0f));
            blurPassRTs[0]->ActivateTexture(gl::GL_TEXTURE0);
            blurPassRTs[1]->ActivateImage(0, 0, gl::GL_WRITE_ONLY);
            OGL_CALL(gl::glDispatchCompute, numGroups.x, numGroups.y, 1);

            base *= 2.0f;
            current = blurPassRTs[1].get();
        }

        numGroups = glm::ivec2(glm::ceil(glm::vec2(sourceRTSize) / groupSize));

        OGL_CALL(gl::glMemoryBarrier, gl::GL_ALL_BARRIER_BITS);
        OGL_SCALL(gl::glFinish);

        combineProgram->UseProgram();
        combineProgram->SetUniform(combineUniformIds[0], 0);
        combineProgram->SetUniform(combineUniformIds[1], 0);
        combineProgram->SetUniform(combineUniformIds[2], blurTextureUnitIds);
        combineProgram->SetUniform(combineUniformIds[3], params.defocus);
        combineProgram->SetUniform(combineUniformIds[4], params.bloomIntensity);
        sourceRT->ActivateTexture(gl::GL_TEXTURE0);
        for (unsigned int i = 0; i < NUM_PASSES; ++i) {
            blurRTs[i][1]->ActivateTexture(gl::GL_TEXTURE1 + i);
        }
        targetRT->ActivateImage(0, 0, gl::GL_WRITE_ONLY);
        OGL_CALL(gl::glDispatchCompute, numGroups.x, numGroups.y, 1);
        OGL_CALL(gl::glMemoryBarrier, gl::GL_ALL_BARRIER_BITS);
        OGL_SCALL(gl::glFinish);
    }

    void BloomEffect::Resize(const glm::uvec2& screenSize)
    {
        blurTextureUnitIds.clear();
        sourceRTSize = screenSize;
        TextureDescriptor texDesc{ 16, gl::GL_RGBA32F, gl::GL_RGBA, gl::GL_FLOAT };
        glm::uvec2 size(screenSize.x / 2, screenSize.y / 2);
        glaresRT.reset(new GLTexture(size.x, size.y, texDesc, nullptr));

        unsigned int base = 1;
        auto blurTexUnit = 0;
        for (auto& blurPassRTs : blurRTs) {
            blurTextureUnitIds.emplace_back(++blurTexUnit);
            glm::uvec2 sizeBlurRT(glm::max(size.x / base, 1u), glm::max(size.y / base, 1u));
            blurPassRTs[0].reset(new GLTexture(sizeBlurRT.x, sizeBlurRT.y, texDesc, nullptr));
            blurPassRTs[1].reset(new GLTexture(sizeBlurRT.x, sizeBlurRT.y, texDesc, nullptr));
            base *= 2;
        }
    }
}
