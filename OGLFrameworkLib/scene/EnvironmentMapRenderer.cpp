/**
 * @file   EnvironmentMapRenderer.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.09.08
 *
 * @brief  Implementation of a renderer for environment maps.
 */

#include "EnvironmentMapRenderer.h"
#include <app/ApplicationBase.h>

namespace cgu {

    EnvironmentMapRenderer::EnvironmentMapRenderer(ApplicationBase* app) :
        envMapProgram_(app->GetGPUProgramManager()->GetResource("shader/screenQuad.vp|shader/envmap/drawEnvMap.fp")),
        envMapUniformIds_(envMapProgram_->GetUniformLocations({ "envMapTex", "vpInv", "camPos" })),
        screenQuad_(*app->GetScreenQuadRenderable())
    {
    }

    EnvironmentMapRenderer::~EnvironmentMapRenderer() = default;

    void EnvironmentMapRenderer::Draw(const PerspectiveCamera& camera, const GLTexture& tex)
    {
        Draw(camera.GetPosition(), camera.GetViewMatrix(), camera.GetProjMatrix(), tex);
    }

    void EnvironmentMapRenderer::Draw(const glm::vec3& camPos, const glm::mat4& view, const glm::mat4& proj, const GLTexture& tex)
    {
        envMapProgram_->UseProgram();
        tex.ActivateTexture(GL_TEXTURE0);
        envMapProgram_->SetUniform(envMapUniformIds_[0], 0);
        envMapProgram_->SetUniform(envMapUniformIds_[1], glm::inverse(proj * view));
        envMapProgram_->SetUniform(envMapUniformIds_[2], camPos);
        screenQuad_.Draw();
    }
}
