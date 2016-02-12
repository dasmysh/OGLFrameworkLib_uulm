/**
 * @file   DepthOfField.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.02.11
 *
 * @brief  Implementation of the depth of field effect.
 */

#include "DepthOfField.h"
#include "app/ApplicationBase.h"
#include "app/GLWindow.h"
#include <imgui.h>
#include "gfx/CameraView.h"

namespace cgu {

    DepthOfField::DepthOfField(const glm::ivec2 sourceSize, ApplicationBase* app) :
        cocProgram(app->GetGPUProgramManager()->GetResource("shader/dof/coc.cp")),
        cocUniformIds(cocProgram->GetUniformLocations({ "colorTex", "depthTex", "targetTex", "focusZ", "scale", "clipInfo" })),
        hBlurProgram(app->GetGPUProgramManager()->GetResource("shader/dof/blurDoF.cp,HORIZONTAL")),
        hBlurUniformIds(hBlurProgram->GetUniformLocations({ "sourceTex", "targetFrontTex", "targetBackTex", "maxCoCRadius", "frontBlurRadius", "invFrontBlurRadius" })),
        vBlurProgram(app->GetGPUProgramManager()->GetResource("shader/dof/blurDoF.cp")),
        vBlurUniformIds(vBlurProgram->GetUniformLocations({ "sourceFrontTex", "sourceTex", "targetFrontTex", "targetBackTex", "maxCoCRadius", "frontBlurRadius", "invFrontBlurRadius" })),
        combineProgram(app->GetGPUProgramManager()->GetResource("shader/dof/combineDoF.cp")),
        combineUniformIds(combineProgram->GetUniformLocations({ "cocTex", "sourceFrontTex", "sourceBackTex", "targetTex" })),
        sourceRTSize(sourceSize)
    {
        params.focusZ = 0.5f;
        params.lensRadius = 0.01f;

        Resize(sourceSize);
    }

    DepthOfField::~DepthOfField() = default;

    void DepthOfField::RenderParameterSliders()
    {
        if (ImGui::TreeNode("DepthOfField Parameters"))
        {
            ImGui::InputFloat("DoF Focus", &params.focusZ, 0.001f);
            ImGui::InputFloat("Lens Radius", &params.lensRadius, 0.01f);
            ImGui::TreePop();
        }
    }

    void DepthOfField::ApplyEffect(const CameraView& cam, const GLTexture* color, const GLTexture* depth, const GLTexture* targetRT)
    {
        const glm::vec2 groupSize{ 32.0f, 16.0f };

        auto targetSize = glm::vec2(sourceRTSize);
        auto numGroups = glm::ivec2(glm::ceil(targetSize / groupSize));


        auto maxCoCRadius = CalculateMaxCoCRadius(cam);
        auto nearBlurRadius = static_cast<int>(glm::ceil(glm::max(static_cast<float>(sourceRTSize.y) / 100.0f, 12.0f)));
        auto invNearBlurRadius = 1.0f / glm::max(static_cast<float>(nearBlurRadius), 0.0001f);
        const auto scale = CalculateImagePlanePixelsPerMeter(cam) * params.lensRadius / (params.focusZ * maxCoCRadius);
        glm::vec3 clipInfo(cam.GetNearZ() * cam.GetFarZ(), cam.GetNearZ() - cam.GetFarZ(), cam.GetFarZ());

        cocProgram->UseProgram();
        cocProgram->SetUniform(cocUniformIds[0], 0);
        cocProgram->SetUniform(cocUniformIds[1], 1);
        cocProgram->SetUniform(cocUniformIds[2], 0);
        cocProgram->SetUniform(cocUniformIds[3], params.focusZ);
        cocProgram->SetUniform(cocUniformIds[4], scale);
        cocProgram->SetUniform(cocUniformIds[5], clipInfo);
        color->ActivateTexture(GL_TEXTURE0);
        depth->ActivateTexture(GL_TEXTURE1);
        cocRT->ActivateImage(0, 0, GL_WRITE_ONLY);
        OGL_CALL(glDispatchCompute, numGroups.x, numGroups.y, 1);
        OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
        OGL_SCALL(glFinish);


        hBlurProgram->UseProgram();
        hBlurProgram->SetUniform(hBlurUniformIds[0], 0);
        hBlurProgram->SetUniform(hBlurUniformIds[1], 0);
        hBlurProgram->SetUniform(hBlurUniformIds[2], 1);
        hBlurProgram->SetUniform(hBlurUniformIds[3], maxCoCRadius);
        hBlurProgram->SetUniform(hBlurUniformIds[4], nearBlurRadius);
        hBlurProgram->SetUniform(hBlurUniformIds[5], invNearBlurRadius);
        cocRT->ActivateTexture(GL_TEXTURE0);
        blurRTs[0][0]->ActivateImage(0, 0, GL_WRITE_ONLY);;
        blurRTs[0][1]->ActivateImage(0, 0, GL_WRITE_ONLY);
        OGL_CALL(glDispatchCompute, numGroups.x / 4, numGroups.y, 1);
        OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
        OGL_SCALL(glFinish);

        vBlurProgram->UseProgram();
        vBlurProgram->SetUniform(vBlurUniformIds[0], 0);
        vBlurProgram->SetUniform(vBlurUniformIds[1], 1);
        vBlurProgram->SetUniform(vBlurUniformIds[2], 0);
        vBlurProgram->SetUniform(vBlurUniformIds[3], 1);
        vBlurProgram->SetUniform(vBlurUniformIds[4], maxCoCRadius);
        vBlurProgram->SetUniform(vBlurUniformIds[5], nearBlurRadius);
        vBlurProgram->SetUniform(vBlurUniformIds[6], invNearBlurRadius);
        blurRTs[0][0]->ActivateTexture(GL_TEXTURE0);
        blurRTs[0][1]->ActivateTexture(GL_TEXTURE1);
        blurRTs[1][0]->ActivateImage(0, 0, GL_WRITE_ONLY);
        blurRTs[1][1]->ActivateImage(1, 0, GL_WRITE_ONLY);
        OGL_CALL(glDispatchCompute, numGroups.x / 4, numGroups.y / 4, 1);
        OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
        OGL_SCALL(glFinish);

        combineProgram->UseProgram();
        combineProgram->SetUniform(combineUniformIds[0], 0);
        combineProgram->SetUniform(combineUniformIds[1], 1);
        combineProgram->SetUniform(combineUniformIds[2], 2);
        combineProgram->SetUniform(combineUniformIds[3], 0);
        /*combineProgram->SetUniform(combineUniformIds[2], blurTextureUnitIds);
        combineProgram->SetUniform(combineUniformIds[3], params.defocus);
        combineProgram->SetUniform(combineUniformIds[4], params.bloomIntensity);
        sourceRT->GetTextures()[0]->ActivateTexture(GL_TEXTURE0);
        for (unsigned int i = 0; i < NUM_PASSES; ++i) {
            blurRTs[i][1]->ActivateTexture(GL_TEXTURE1 + i);
        }*/
        cocRT->ActivateTexture(GL_TEXTURE0);
        blurRTs[1][0]->ActivateTexture(GL_TEXTURE1);
        blurRTs[1][1]->ActivateTexture(GL_TEXTURE2);
        targetRT->ActivateImage(0, 0, GL_WRITE_ONLY);
        OGL_CALL(glDispatchCompute, numGroups.x, numGroups.y, 1);
        OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
        OGL_SCALL(glFinish);
    }

    void DepthOfField::Resize(const glm::uvec2& screenSize)
    {
        sourceRTSize = screenSize;
        TextureDescriptor texDesc{ 16, GL_RGBA32F, GL_RGBA, GL_FLOAT };
        glm::uvec2 size(screenSize.x, screenSize.y);
        cocRT.reset(new GLTexture(size.x, size.y, texDesc, nullptr));

        blurRTs[0][0].reset(new GLTexture(size.x / 4, size.y, texDesc, nullptr));
        blurRTs[0][1].reset(new GLTexture(size.x / 4, size.y, texDesc, nullptr));
        blurRTs[1][0].reset(new GLTexture(size.x / 4, size.y / 4, texDesc, nullptr));
        blurRTs[1][1].reset(new GLTexture(size.x / 4, size.y / 4, texDesc, nullptr));
    }

    float DepthOfField::CalculateImagePlanePixelsPerMeter(const CameraView& cam) const
    {
        const auto scale = -2.0f * glm::tan(cam.GetFOV() * 0.5f);
        return static_cast<float>(sourceRTSize.y) / scale;
    }

    float DepthOfField::CalculateCoCRadius(const CameraView& cam, float z) const
    {
        // TODO: this returns negative values. [2/12/2016 Sebastian Maisch]
        return CalculateImagePlanePixelsPerMeter(cam) * ((z - params.focusZ) * params.lensRadius / params.focusZ) / -z;
    }

    int DepthOfField::CalculateMaxCoCRadius(const CameraView& cam) const
    {
        auto maxR = glm::max(CalculateCoCRadius(cam, cam.GetNearZ()), CalculateCoCRadius(cam, cam.GetFarZ()));
        return static_cast<int>(glm::ceil(glm::min(maxR, sourceRTSize.x * 0.02f)));
    }
}
