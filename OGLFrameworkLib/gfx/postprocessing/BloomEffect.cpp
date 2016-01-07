/**
 * @file   BloomEffect.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.12.22
 *
 * @brief  Implementation of the bloom effect.
 */

#include "BloomEffect.h"
#include "app/ApplicationBase.h"
#include "gfx/glrenderer/GLRenderTarget.h"
#include "app/GLWindow.h"

namespace cgu {

    BloomEffect::BloomEffect(ApplicationBase* app) :
        renderable(app->GetScreenQuadRenderable()),
        glareDetectProgram(app->GetGPUProgramManager()->GetResource("shader/tm/glareDetect.cp")),
        glareUniformIds(glareDetectProgram->GetUniformLocations({ "sourceTex", "targetTex", "exposure", "bloomThreshold" })),
        blurProgram(app->GetGPUProgramManager()->GetResource("shader/tm/blurBloom.cp")),
        blurUniformIds(blurProgram->GetUniformLocations({ "sourceTex", "targetTex", "dir", "bloomWidth" })),
        combineProgram(nullptr),
        combineUniformIds(),
        sourceRTSize(app->GetWindow()->GetWidth(), app->GetWindow()->GetHeight())
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
        TextureDescriptor texDesc{ 128, GL_RGBA32F, GL_RGBA, GL_FLOAT };
        glm::uvec2 size(app->GetWindow()->GetWidth() / 2, app->GetWindow()->GetHeight() / 2);
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

    BloomEffect::~BloomEffect() = default;

    void BloomEffect::AddParameterSlidersToBar(TwBar* bar)
    {
        TwAddVarRW(bar, "BLOOM_THRESH", TW_TYPE_FLOAT, &params.bloomThreshold, " label='Bloom Threshold' min=0.3 max=2.0 step=0.01");
        TwAddVarRW(bar, "BLOOM_WIDTH", TW_TYPE_FLOAT, &params.bloomWidth, " label='Bloom Width' min=0.1 max=2 step=0.1");
        TwAddVarRW(bar, "BLOOM_DEFOCUS", TW_TYPE_FLOAT, &params.defocus, " label='Bloom Defocus' min=0.01 max=1 step=0.01");
        TwAddVarRW(bar, "BLOOM_INTENSITY", TW_TYPE_FLOAT, &params.bloomIntensity, " label='Bloom Intensity' min=0.1 max=2 step=0.1");
    }

    void BloomEffect::ApplyEffect(GLRenderTarget* sourceRT, GLRenderTarget* targetRT)
    {
        const glm::vec2 groupSize{ 32.0f, 16.0f };

        auto targetSize = glm::vec2(sourceRTSize) / 2.0f;
        auto numGroups = glm::ivec2(glm::ceil(targetSize / groupSize));
        glareDetectProgram->UseProgram();
        glareDetectProgram->SetUniform(glareUniformIds[0], 0);
        glareDetectProgram->SetUniform(glareUniformIds[1], 0);
        glareDetectProgram->SetUniform(glareUniformIds[2], params.exposure);
        glareDetectProgram->SetUniform(glareUniformIds[3], params.bloomThreshold);
        sourceRT->GetTextures()[0]->ActivateTexture(GL_TEXTURE0);
        glaresRT->ActivateImage(0, 0, GL_WRITE_ONLY);
        OGL_CALL(glDispatchCompute, numGroups.x, numGroups.y, 1);
        OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
        OGL_SCALL(glFinish);

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
            current->ActivateTexture(GL_TEXTURE0);
            blurPassRTs[0]->ActivateImage(0, 0, GL_WRITE_ONLY);
            OGL_CALL(glDispatchCompute, numGroups.x, numGroups.y, 1);
            OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
            OGL_SCALL(glFinish);

            blurProgram->SetUniform(blurUniformIds[2], glm::vec2(0.0f, 1.0f));
            blurPassRTs[0]->ActivateTexture(GL_TEXTURE0);
            blurPassRTs[1]->ActivateImage(0, 0, GL_WRITE_ONLY);
            OGL_CALL(glDispatchCompute, numGroups.x, numGroups.y, 1);

            base *= 2.0f;
            current = blurPassRTs[1].get();
        }

        numGroups = glm::ivec2(glm::ceil(glm::vec2(sourceRTSize) / groupSize));

        OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
        OGL_SCALL(glFinish);

        combineProgram->UseProgram();
        combineProgram->SetUniform(combineUniformIds[0], 0);
        combineProgram->SetUniform(combineUniformIds[1], 0);
        combineProgram->SetUniform(combineUniformIds[2], blurTextureUnitIds);
        combineProgram->SetUniform(combineUniformIds[3], params.defocus);
        combineProgram->SetUniform(combineUniformIds[4], params.bloomIntensity);
        sourceRT->GetTextures()[0]->ActivateTexture(GL_TEXTURE0);
        for (unsigned int i = 0; i < NUM_PASSES; ++i) {
            blurRTs[i][1]->ActivateTexture(GL_TEXTURE1 + i);
        }
        targetRT->GetTextures()[0]->ActivateImage(0, 0, GL_WRITE_ONLY);
        OGL_CALL(glDispatchCompute, numGroups.x, numGroups.y, 1);
        OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
        OGL_SCALL(glFinish);
    }
}
