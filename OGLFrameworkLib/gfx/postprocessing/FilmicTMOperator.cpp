/**
 * @file   FilmicTMOperator.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.12.21
 *
 * @brief  Implementation of the filmic tone-mapping operator.
 */

#include "FilmicTMOperator.h"
#include "app/ApplicationBase.h"
#include "gfx/glrenderer/GLRenderTarget.h"
#include "gfx/glrenderer/GLUniformBuffer.h"
#include "gfx/glrenderer/ScreenQuadRenderable.h"
#include <imgui.h>

namespace cgu {

    FilmicTMOperator::FilmicTMOperator(ApplicationBase* app) :
        tmProgram(app->GetGPUProgramManager()->GetResource("shader/screenQuad.vp|shader/tm/filmic.fp")),
        renderable(app->GetScreenQuadRenderable()),
        uniformIds(tmProgram->GetUniformLocations({ "sourceTex" })),
        filmicUBO(new GLUniformBuffer("filmicBuffer", sizeof(FilmicTMParameters), app->GetUBOBindingPoints()))
    {
        params.sStrength = 0.15f;
        params.linStrength = 0.5f;
        params.linAngle = 0.1f;
        params.toeStrength = 0.2f;
        params.toeNumerator = 0.02f;
        params.toeDenominator = 0.3f;
        params.white = 11.2f;
        params.exposure = 2.0f;

        tmProgram->BindUniformBlock("filmicBuffer", *app->GetUBOBindingPoints());

        // Alternative values:
        /*params.sStrength = 0.22f;
        params.linStrength = 0.3f;
        params.linAngle = 0.1f;
        params.toeStrength = 0.2f;
        params.toeNumerator = 0.1f;
        params.toeDenominator = 0.3f;
        params.white = 11.2f;
        params.gamma = 2.2f;*/
    }

    /** Default destructor. */
    FilmicTMOperator::~FilmicTMOperator() = default;

    void FilmicTMOperator::RenderParameterSliders()
    {
        ImGui::InputFloat("Shoulder Strength", &params.sStrength, 0.01f);
        ImGui::InputFloat("Linear Strength", &params.linStrength, 0.1f);
        ImGui::InputFloat("Linear Angle", &params.linAngle, 0.01f);
        ImGui::InputFloat("Toe Strength", &params.toeStrength, 0.1f);
        ImGui::InputFloat("Toe Numerator", &params.toeNumerator, 0.01f);
        ImGui::InputFloat("Toe Denominator", &params.toeDenominator, 0.1f);
        ImGui::InputFloat("White", &params.white, 0.1f);
        // no gamma on sRGB targets
        // TwAddVarRW(bar, "Gamma", TW_TYPE_FLOAT, &params.gamma, " label='Gamma' min=1.0 max=3.0 step=0.1");
    }

    void FilmicTMOperator::ApplyTonemapping(GLRenderTarget* sourceRT, GLRenderTarget* targetRT)
    {
        filmicUBO->UploadData(0, sizeof(FilmicTMParameters), &params);
        filmicUBO->BindBuffer();

        targetRT->BatchDraw([this, sourceRT](cgu::GLBatchRenderTarget & brt) {
            tmProgram->UseProgram();
            sourceRT->GetTextures()[0]->ActivateTexture(GL_TEXTURE0);
            tmProgram->SetUniform(uniformIds[0], 0);
            renderable->Draw();
        });
    }
}
