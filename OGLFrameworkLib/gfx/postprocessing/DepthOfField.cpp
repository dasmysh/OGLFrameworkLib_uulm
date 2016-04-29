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
#include "gfx/ArcballCamera.h"

namespace cgu {

    DepthOfField::DepthOfField(const glm::ivec2 sourceSize, ApplicationBase* app) :
        cocProgram(app->GetGPUProgramManager()->GetResource("shader/dof/coc.cp")),
        cocUniformIds(cocProgram->GetUniformLocations({ "colorTex", "depthTex", "targetTex", "focusZ", "scale", "clipInfo" })),
        combineProgram(app->GetGPUProgramManager()->GetResource("shader/dof/combineDoF.cp")),
        combineUniformIds(combineProgram->GetUniformLocations({ "cocTex", "sourceFrontTex", "sourceBackTex", "targetTex" })),
        debugProgram(app->GetGPUProgramManager()->GetResource("shader/screenQuad.vp|shader/debugTexOut.fp")),
        debugUniformIds(debugProgram->GetUniformLocations({ "sourceTex" })),
        sourceRTSize(sourceSize)
    {
        params.focusZ = 2.3f;
        params.apertureRadius = 0.001f;

        std::stringstream shaderDefines;
        shaderDefines << "SIZE_FACTOR " << RT_SIZE_FACTOR;
        hBlurProgram = app->GetGPUProgramManager()->GetResource("shader/dof/blurDoF.cp,HORIZONTAL," + shaderDefines.str());
        hBlurUniformIds = hBlurProgram->GetUniformLocations({ "sourceTex", "targetFrontTex", "targetBackTex", "maxCoCRadius", "frontBlurRadius", "invFrontBlurRadius" });
        vBlurProgram = app->GetGPUProgramManager()->GetResource("shader/dof/blurDoF.cp," + shaderDefines.str());
        vBlurUniformIds = vBlurProgram->GetUniformLocations({ "sourceFrontTex", "sourceTex", "targetFrontTex", "targetBackTex", "maxCoCRadius", "frontBlurRadius", "invFrontBlurRadius" });

        FrameBufferDescriptor hdrFBODesc;
        hdrFBODesc.texDesc_.emplace_back(TextureDescriptor{ 16, GL_RGBA32F, GL_RGBA, GL_FLOAT });
        hdrFBODesc.texDesc_.emplace_back(TextureDescriptor{ 4, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT });
        debugRT.reset(new GLRenderTarget(app->GetWindow()->GetWidth(), app->GetWindow()->GetHeight(), hdrFBODesc));
        debugRenderable = app->GetScreenQuadRenderable();

        Resize(sourceSize);
    }

    DepthOfField::~DepthOfField() = default;

    void DepthOfField::RenderParameterSliders()
    {
        if (ImGui::TreeNode("DepthOfField Parameters"))
        {
            ImGui::InputFloat("DoF Focus", &params.focusZ, 0.01f);
            ImGui::InputFloat("Aperture Radius", &params.apertureRadius, 0.0001f);
            ImGui::TreePop();
        }
    }

    void DepthOfField::ApplyEffect(const ArcballCamera& cam, const GLTexture* color, const GLTexture* depth, const GLTexture* targetRT)
    {
        const glm::vec2 groupSize{ 32.0f, 16.0f };

        auto targetSize = glm::vec2(sourceRTSize);
        auto numGroups = glm::ivec2(glm::ceil(targetSize / groupSize));

        auto focalLength = CalculateFocalLength(cam) * targetSize.y;
        auto maxCoCRadius = CalculateMaxCoCRadius(cam);
        auto imaxCoCRadius = static_cast<int>(maxCoCRadius);
        auto nearBlurRadius = static_cast<int>(glm::ceil(glm::max(static_cast<float>(sourceRTSize.y) / 100.0f, 12.0f)));
        auto invNearBlurRadius = 1.0f / glm::max(static_cast<float>(nearBlurRadius), 0.0001f);
        // const auto scale = (params.apertureRadius * focalLength) / (params.focusZ - focalLength);
        const auto scale = (params.apertureRadius * focalLength) / (params.focusZ * maxCoCRadius);
        glm::vec3 clipInfo(2.0f * cam.GetNearZ() * cam.GetFarZ(), cam.GetFarZ() - cam.GetNearZ(), cam.GetFarZ() + cam.GetNearZ());

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

        float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        debugRT->BatchDraw([this,&clearColor](GLBatchRenderTarget& brt)
        {
            brt.Clear(static_cast<unsigned int>(cgu::ClearFlags::CF_RenderTarget) | static_cast<unsigned int>(cgu::ClearFlags::CF_Depth), clearColor, 1.0, 0);
            debugProgram->UseProgram();
            cocRT->ActivateTexture(GL_TEXTURE0);
            debugProgram->SetUniform(debugUniformIds[0], 0);
            debugRenderable->Draw();
        });

        hBlurProgram->UseProgram();
        hBlurProgram->SetUniform(hBlurUniformIds[0], 0);
        hBlurProgram->SetUniform(hBlurUniformIds[1], 0);
        hBlurProgram->SetUniform(hBlurUniformIds[2], 1);
        hBlurProgram->SetUniform(hBlurUniformIds[3], imaxCoCRadius);
        hBlurProgram->SetUniform(hBlurUniformIds[4], nearBlurRadius);
        hBlurProgram->SetUniform(hBlurUniformIds[5], invNearBlurRadius);
        cocRT->ActivateTexture(GL_TEXTURE0);
        blurRTs[0][0]->ActivateImage(0, 0, GL_WRITE_ONLY);;
        blurRTs[0][1]->ActivateImage(1, 0, GL_WRITE_ONLY);
        OGL_CALL(glDispatchCompute, numGroups.x / RT_SIZE_FACTOR, numGroups.y, 1);
        OGL_CALL(glMemoryBarrier, GL_ALL_BARRIER_BITS);
        OGL_SCALL(glFinish);

        debugRT->BatchDraw([this, &clearColor](GLBatchRenderTarget& brt)
        {
            brt.Clear(static_cast<unsigned int>(cgu::ClearFlags::CF_RenderTarget) | static_cast<unsigned int>(cgu::ClearFlags::CF_Depth), clearColor, 1.0, 0);
            debugProgram->UseProgram();
            blurRTs[0][0]->ActivateTexture(GL_TEXTURE0);
            debugProgram->SetUniform(debugUniformIds[0], 0);
            debugRenderable->Draw();
        });

        vBlurProgram->UseProgram();
        vBlurProgram->SetUniform(vBlurUniformIds[0], 0);
        vBlurProgram->SetUniform(vBlurUniformIds[1], 1);
        vBlurProgram->SetUniform(vBlurUniformIds[2], 0);
        vBlurProgram->SetUniform(vBlurUniformIds[3], 1);
        vBlurProgram->SetUniform(vBlurUniformIds[4], imaxCoCRadius);
        vBlurProgram->SetUniform(vBlurUniformIds[5], nearBlurRadius);
        vBlurProgram->SetUniform(vBlurUniformIds[6], invNearBlurRadius);
        blurRTs[0][0]->ActivateTexture(GL_TEXTURE0);
        blurRTs[0][1]->ActivateTexture(GL_TEXTURE1);
        blurRTs[1][0]->ActivateImage(0, 0, GL_WRITE_ONLY);
        blurRTs[1][1]->ActivateImage(1, 0, GL_WRITE_ONLY);
        OGL_CALL(glDispatchCompute, numGroups.x / RT_SIZE_FACTOR, numGroups.y / RT_SIZE_FACTOR, 1);
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

        blurRTs[0][0].reset(new GLTexture(size.x / RT_SIZE_FACTOR, size.y, texDesc, nullptr));
        blurRTs[0][1].reset(new GLTexture(size.x / RT_SIZE_FACTOR, size.y, texDesc, nullptr));
        blurRTs[1][0].reset(new GLTexture(size.x / RT_SIZE_FACTOR, size.y / RT_SIZE_FACTOR, texDesc, nullptr));
        blurRTs[1][1].reset(new GLTexture(size.x / RT_SIZE_FACTOR, size.y / RT_SIZE_FACTOR, texDesc, nullptr));

        debugRT->Resize(screenSize.x, screenSize.y);
    }

    float DepthOfField::CalculateFocalLength(const ArcballCamera& cam) const
    {
        const auto scale = 2.0f * glm::tan(cam.GetFOV() * 0.5f);
        return 1.0f / scale;
        // return static_cast<float>(sourceRTSize.y) / scale;
    }

    float DepthOfField::CalculateCoCRadius(const ArcballCamera& cam, float z) const
    {
        // TODO: this returns negative values. [2/12/2016 Sebastian Maisch]
        auto focalLength = CalculateFocalLength(cam);
        // const float rzmeters = (z - params.focusZ) * params.apertureRadius / (-params.focusZ);
        // const float rimeters = rzmeters / z;
        auto resultMeters = (glm::abs(z - params.focusZ) * params.apertureRadius * focalLength) / (z * (params.focusZ - focalLength));
        return resultMeters;
    }

    float DepthOfField::CalculateMaxCoCRadius(const ArcballCamera& cam) const
    {
        auto maxR = glm::max(CalculateCoCRadius(cam, cam.GetNearZ()), CalculateCoCRadius(cam, cam.GetFarZ()));
        return glm::ceil(glm::min(sourceRTSize.y * maxR, sourceRTSize.x * 0.02f));
    }
}
