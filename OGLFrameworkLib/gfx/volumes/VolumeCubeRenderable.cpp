/**
 * @file   VolumeCubeRenderable.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.08.19
 *
 * @brief  Implementation of a renderable used to determine starting and end positions for raycasting.
 */

#include "VolumeCubeRenderable.h"
#include "gfx/glrenderer/GPUProgram.h"
#include "app/ApplicationBase.h"

namespace cgu {

    /**
     *  Constructor.
     *  @param drawProg the GPU program for drawing the volume
     *  @param app the application object
     */
    VolumeCubeRenderable::VolumeCubeRenderable(GPUProgram* drawProg, ApplicationBase* app) :
        vBuffer(0),
        iBuffer(0),
        backProgram(app->GetGPUProgramManager()->GetResource("renderCubeCoordinates.vp|renderCubeCoordinates.fp")),
        drawProgram(drawProg),
        application(app)
    {
        backProgram->BindUniformBlock(perspectiveProjectionUBBName, *app->GetUBOBindingPoints());
        backAttribBinds.GetUniformIds() = backProgram->GetUniformLocations({ "minTexCoords", "maxTexCoords" });

        drawProgram->UseProgram();
        drawProgram->BindUniformBlock(perspectiveProjectionUBBName, *app->GetUBOBindingPoints());
        drawAttribBinds.GetUniformIds() = drawProgram->GetUniformLocations({ "volume", "transferFunc", "back",
            "minTexCoords", "maxTexCoords", "stepSize", "lodLevel" });
        drawProgram->SetUniform(drawAttribBinds.GetUniformIds()[0], 0);
        drawProgram->SetUniform(drawAttribBinds.GetUniformIds()[1], 1);
        drawProgram->SetUniform(drawAttribBinds.GetUniformIds()[2], 0);

        CreateVertexIndexBuffers();
    }

    /**
     *  Copy-Constructor.
     *  @param orig the original VolumeCubeRenderable
     */
    VolumeCubeRenderable::VolumeCubeRenderable(const VolumeCubeRenderable& orig) :
        VolumeCubeRenderable(orig.drawProgram, orig.application)
    {
    }

    /**
     *  Move-Constructor.
     *  @param orig the original VolumeCubeRenderable
     */
    VolumeCubeRenderable::VolumeCubeRenderable(VolumeCubeRenderable&& orig) :
        vBuffer(orig.vBuffer),
        iBuffer(orig.iBuffer),
        backProgram(orig.backProgram),
        backAttribBinds(std::move(orig.backAttribBinds)),
        drawProgram(orig.drawProgram),
        drawAttribBinds(std::move(orig.drawAttribBinds)),
        application(orig.application)

    {
        orig.vBuffer = 0;
        orig.iBuffer = 0;
        orig.backProgram = nullptr;
        orig.drawProgram = nullptr;
    }

    /**
     *  Assignment operator.
     *  @param orig the original VolumeCubeRenderable
     *  @return the newly assigned VolumeCubeRenderable
     */
    VolumeCubeRenderable& VolumeCubeRenderable::operator=(const VolumeCubeRenderable& orig)
    {
        VolumeCubeRenderable tmp{ orig };
        std::swap(*this, tmp);
        return *this;
    }

    /**
     *  Move-Assignment operator.
     *  @param orig the original VolumeCubeRenderable
     *  @return the newly assigned VolumeCubeRenderable
     */
    VolumeCubeRenderable& VolumeCubeRenderable::operator=(VolumeCubeRenderable&& orig)
    {
        this->~VolumeCubeRenderable();
        backAttribBinds = std::move(orig.backAttribBinds);
        drawAttribBinds = std::move(orig.drawAttribBinds);
        backProgram = orig.backProgram;
        orig.backProgram = nullptr;
        drawProgram = orig.drawProgram;
        orig.drawProgram = nullptr;
        vBuffer = orig.vBuffer;
        orig.vBuffer = 0;
        iBuffer = orig.iBuffer;
        orig.iBuffer = 0;
        return *this;
    }

    /**
     *  Destructor.
     */
    VolumeCubeRenderable::~VolumeCubeRenderable()
    {
        DeleteVertexIndexBuffers();
    }

    /**
     *  Creates the vertex- and index-buffers and their vertex attribute bindings.
     */
    void VolumeCubeRenderable::CreateVertexIndexBuffers()
    {
        std::vector<VolumeCubeVertex> vertices;
        vertices.push_back(VolumeCubeVertex{ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f) });
        vertices.push_back(VolumeCubeVertex{ glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f) });
        vertices.push_back(VolumeCubeVertex{ glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f) });
        vertices.push_back(VolumeCubeVertex{ glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec3(1.0f, 1.0f, 0.0f) });
        vertices.push_back(VolumeCubeVertex{ glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f) });
        vertices.push_back(VolumeCubeVertex{ glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 1.0f) });
        vertices.push_back(VolumeCubeVertex{ glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 1.0f) });
        vertices.push_back(VolumeCubeVertex{ glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f) });

        OGL_CALL(glGenBuffers, 1, &vBuffer);
        OGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, vBuffer);
        OGL_CALL(glBufferData, GL_ARRAY_BUFFER, 8 * sizeof(VolumeCubeVertex), vertices.data(), GL_STATIC_DRAW);

        unsigned int indexData[36] = {
            1, 0, 3, 3, 0, 2,
            4, 5, 6, 6, 5, 7,
            0, 1, 4, 4, 1, 5,
            3, 2, 7, 7, 2, 6,
            1, 3, 5, 5, 3, 7,
            0, 4, 2, 2, 4, 6
        };
        OGL_CALL(glGenBuffers, 1, &iBuffer);
        OGL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, iBuffer);
        OGL_CALL(glBufferData, GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(unsigned int), indexData, GL_STATIC_DRAW);

        FillVertexAttributeBindings(backProgram, backAttribBinds);
        FillVertexAttributeBindings(drawProgram, drawAttribBinds);
    }

    /**
     *  Deletes the vertex- and index-buffers.
     */
    void VolumeCubeRenderable::DeleteVertexIndexBuffers()
    {
        if (vBuffer != 0) {
            OGL_CALL(glDeleteBuffers, 1, &vBuffer);
            vBuffer = 0;
        }

        if (iBuffer != 0) {
            OGL_CALL(glDeleteBuffers, 1, &iBuffer);
            iBuffer = 0;
        }
    }

    /**
     *  Draws the volumes back faces.
     */
    void VolumeCubeRenderable::DrawBack(const glm::vec4& texMin, const glm::vec4& texMax) const
    {
        glCullFace(GL_FRONT);
        backProgram->UseProgram();
        backProgram->SetUniform(backAttribBinds.GetUniformIds()[0], texMin);
        backProgram->SetUniform(backAttribBinds.GetUniformIds()[1], texMax);
        Draw(backProgram, backAttribBinds);
    }

    /**
     *  Draws the volume.
     */
    void VolumeCubeRenderable::Draw(float stepSize, float mipLevel, const glm::vec4& texMin, const glm::vec4& texMax) const
    {
        glCullFace(GL_BACK);
        drawProgram->UseProgram();
        drawProgram->SetUniform(drawAttribBinds.GetUniformIds()[3], texMin);
        drawProgram->SetUniform(drawAttribBinds.GetUniformIds()[4], texMax);
        drawProgram->SetUniform(drawAttribBinds.GetUniformIds()[5], stepSize);
        drawProgram->SetUniform(drawAttribBinds.GetUniformIds()[6], mipLevel);
        Draw(drawProgram, drawAttribBinds);
    }


    /**
     *  Internal rendering method.
     *  @param attribBinding the vertex attribute bindings to use.
     */
    // ReSharper disable once CppMemberFunctionMayBeStatic
    void VolumeCubeRenderable::Draw(GPUProgram* program, const ShaderMeshAttributes& attribBinds) const
    {
        attribBinds.GetVertexAttributes()[0]->EnableVertexAttributeArray();
        OGL_CALL(glDrawElements, GL_TRIANGLES, 36, GL_UNSIGNED_INT, static_cast<char*> (nullptr));
        attribBinds.GetVertexAttributes()[0]->DisableVertexAttributeArray();
    }

    /**
     *  Fills the vertex attribute bindings.
     *  @param program the program to create bindings for.
     *  @param attribBinds the created bindings.
     */
    void VolumeCubeRenderable::FillVertexAttributeBindings(GPUProgram* program, ShaderMeshAttributes& attribBinds) const
    {
        assert(attribBinds.GetVertexAttributes().size() == 0);

        auto loc = program->GetAttributeLocations({ "position", "texPosition" });
        OGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, vBuffer);
        attribBinds.GetVertexAttributes().push_back(program->CreateVertexAttributeArray(vBuffer, iBuffer));
        attribBinds.GetVertexAttributes()[0]->StartAttributeSetup();
        attribBinds.GetVertexAttributes()[0]->AddVertexAttribute(loc[0], 4, GL_FLOAT, GL_FALSE, sizeof(VolumeCubeVertex), offsetof(VolumeCubeVertex, pos));
        attribBinds.GetVertexAttributes()[0]->AddVertexAttribute(loc[1], 3, GL_FLOAT, GL_FALSE, sizeof(VolumeCubeVertex), offsetof(VolumeCubeVertex, posTex));
        attribBinds.GetVertexAttributes()[0]->EndAttributeSetup();
        OGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, 0);
    }
}
