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
    VolumeCubeRenderable::VolumeCubeRenderable(std::shared_ptr<GPUProgram> drawProg, ApplicationBase* app) :
        backProgram(app->GetGPUProgramManager()->GetResource("shader/volume/renderCubeCoordinates.vp|shader/volume/renderCubeCoordinates.fp")),
        drawProgram(std::move(drawProg)),
        application(app)
    {
        backProgram->BindUniformBlock(perspectiveProjectionUBBName, *app->GetUBOBindingPoints());

        drawProgram->UseProgram();
        drawProgram->BindUniformBlock(perspectiveProjectionUBBName, *app->GetUBOBindingPoints());
        drawAttribBinds.GetUniformIds() = drawProgram->GetUniformLocations({ "volume", "transferFunc", "back",
            "stepSize", "lodLevel" });
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
        vBuffer(std::move(orig.vBuffer)),
        iBuffer(std::move(orig.iBuffer)),
        backProgram(std::move(orig.backProgram)),
        backAttribBinds(std::move(orig.backAttribBinds)),
        drawProgram(std::move(orig.drawProgram)),
        drawAttribBinds(std::move(orig.drawAttribBinds)),
        application(orig.application)

    {
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
        backProgram = std::move(orig.backProgram);
        drawProgram = std::move(orig.drawProgram);
        vBuffer = std::move(orig.vBuffer);
        iBuffer = std::move(orig.iBuffer);
        return *this;
    }

    /**
     *  Destructor.
     */
    VolumeCubeRenderable::~VolumeCubeRenderable() = default;

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

        OGL_CALL(gl::glBindBuffer, gl::GL_ARRAY_BUFFER, vBuffer);
        OGL_CALL(gl::glBufferData, gl::GL_ARRAY_BUFFER, 8 * sizeof(VolumeCubeVertex), vertices.data(), gl::GL_STATIC_DRAW);

        unsigned int indexData[36] = {
            1, 0, 3, 3, 0, 2,
            4, 5, 6, 6, 5, 7,
            0, 1, 4, 4, 1, 5,
            3, 2, 7, 7, 2, 6,
            1, 3, 5, 5, 3, 7,
            0, 4, 2, 2, 4, 6
        };
        OGL_CALL(gl::glBindBuffer, gl::GL_ELEMENT_ARRAY_BUFFER, iBuffer);
        OGL_CALL(gl::glBufferData, gl::GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(unsigned int), indexData, gl::GL_STATIC_DRAW);

        FillVertexAttributeBindings(backProgram.get(), backAttribBinds);
        FillVertexAttributeBindings(drawProgram.get(), drawAttribBinds);
    }

    /**
     *  Draws the volumes back faces.
     */
    void VolumeCubeRenderable::DrawBack() const
    {
        gl::glCullFace(gl::GL_FRONT);
        backProgram->UseProgram();
        Draw(backProgram.get(), backAttribBinds);
    }

    /**
     *  Draws the volume.
     */
    void VolumeCubeRenderable::Draw(float stepSize, float mipLevel) const
    {
        gl::glCullFace(gl::GL_BACK);
        drawProgram->UseProgram();
        drawProgram->SetUniform(drawAttribBinds.GetUniformIds()[3], stepSize);
        drawProgram->SetUniform(drawAttribBinds.GetUniformIds()[4], mipLevel);
        Draw(drawProgram.get(), drawAttribBinds);
    }


    /**
     *  Internal rendering method.
     *  @param attribBinding the vertex attribute bindings to use.
     */
    // ReSharper disable once CppMemberFunctionMayBeStatic
    void VolumeCubeRenderable::Draw(GPUProgram*, const ShaderMeshAttributes& attribBinds) const
    {
        attribBinds.GetVertexAttributes()[0]->EnableVertexAttributeArray();
        OGL_CALL(gl::glDrawElements, gl::GL_TRIANGLES, 36, gl::GL_UNSIGNED_INT, static_cast<char*> (nullptr));
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
        OGL_CALL(gl::glBindBuffer, gl::GL_ARRAY_BUFFER, vBuffer);
        attribBinds.GetVertexAttributes().push_back(program->CreateVertexAttributeArray(vBuffer, iBuffer));
        attribBinds.GetVertexAttributes()[0]->StartAttributeSetup();
        attribBinds.GetVertexAttributes()[0]->AddVertexAttribute(loc[0], 4, gl::GL_FLOAT, gl::GL_FALSE, sizeof(VolumeCubeVertex), offsetof(VolumeCubeVertex, pos));
        attribBinds.GetVertexAttributes()[0]->AddVertexAttribute(loc[1], 3, gl::GL_FLOAT, gl::GL_FALSE, sizeof(VolumeCubeVertex), offsetof(VolumeCubeVertex, posTex));
        attribBinds.GetVertexAttributes()[0]->EndAttributeSetup();
        OGL_CALL(gl::glBindBuffer, gl::GL_ARRAY_BUFFER, 0);
    }
}
