/**
 * @file   SimpleMeshRenderer.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.08.26
 *
 * @brief  Implementation of the SimpleMeshRenderer class.
 */

#include "SimpleMeshRenderer.h"
#include <app/ApplicationBase.h>
#include "Mesh.h"
#include "AssimpScene.h"

namespace cgu {

    struct SimpleVertex {
        static const int POSITION_DIMENSION = 4;
        static const bool HAS_NORMAL = false;
        static const bool HAS_TANGENTSPACE = false;
        static const int TEXCOORD_DIMENSION = 4;
        static const int NUM_TEXTURECOORDS = 0;
        static const int NUM_COLORS = 0;
        static const int NUM_INDICES = 0;

        glm::vec4 pos = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

        static void GatherAttributeNames(std::vector<std::string>& attribNames)
        {
            attribNames.push_back("position");
        }

        static void VertexAttributeSetup(GLVertexAttributeArray* vao, const std::vector<BindingLocation>& shaderPositions)
        {
            vao->StartAttributeSetup();
            if (shaderPositions[0]->iBinding >= 0) vao->AddVertexAttribute(shaderPositions[0], 4, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), offsetof(SimpleVertex, pos));
            vao->EndAttributeSetup();
        }

        void SetPosition(float p, int dim) { pos[dim] = p; }
        // ReSharper disable CppMemberFunctionMayBeStatic
        // ReSharper disable CppMemberFunctionMayBeConst
        void SetNormal(const glm::vec3&) { }
        void SetTexCoord(float, int, int) { }
        void SetTangent(const glm::vec3&) { }
        void SetBinormal(const glm::vec3&) { }
        void SetColor(const glm::vec4&, int) { }
        void SetIndex(unsigned int, int) { }
        // ReSharper restore CppMemberFunctionMayBeConst
        // ReSharper restore CppMemberFunctionMayBeStatic

        SimpleVertex() {}
        explicit SimpleVertex(const glm::vec4& position) : pos{ position } {}
    };

    SimpleMeshRenderer::SimpleMeshRenderer(ApplicationBase* app) :
        simpleProgram_(app->GetGPUProgramManager()->GetResource("shader/drawSimple.vp|shader/drawSimple.fp"))
    {
        std::vector<SimpleVertex> vertices;
        std::vector<unsigned int> indices;
        std::array<std::string, 6> submeshNames = { "mesh_cone", "mesh_cube", "mesh_cylinder", "mesh_octahedron", "mesh_sphere", "mesh_torus" };
        for (unsigned int i = 0; i < 6; ++i) {
            AssimpScene mesh("meshes/" + submeshNames[i] + ".obj", app);
            std::vector<SimpleVertex> meshVertices;
            mesh.GetVertices<SimpleVertex>(meshVertices);

            const Mesh* m = &mesh;

            submeshInfo_[i].first = static_cast<unsigned>(indices.size());
            submeshInfo_[i].second = static_cast<unsigned>(m->GetIndices().size());

            auto verticesMin = static_cast<unsigned>(vertices.size());
            vertices.reserve(vertices.size() + meshVertices.size());
            vertices.insert(vertices.end(), meshVertices.begin(), meshVertices.end());

            indices.reserve(indices.size() + m->GetIndices().size());
            for (unsigned int j = 0; j < m->GetIndices().size(); ++j) {
                indices.push_back(m->GetIndices()[j] + verticesMin);
            }
        }

        submeshInfo_[6].first = static_cast<unsigned>(indices.size());
        submeshInfo_[6].second = 1;
        submeshInfo_[7].first = static_cast<unsigned>(indices.size());
        submeshInfo_[7].second = 2;
        indices.push_back(static_cast<unsigned>(vertices.size()));
        indices.push_back(static_cast<unsigned>(vertices.size() + 1));
        vertices.emplace_back(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        vertices.emplace_back(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

        vBuffer_ = std::make_unique<GLBuffer>(GL_STATIC_DRAW);
        OGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, vBuffer_->GetBuffer());
        vBuffer_->InitializeData(vertices);
        OGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, 0);

        iBuffer_ = std::make_unique<GLBuffer>(GL_STATIC_DRAW);
        OGL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, iBuffer_->GetBuffer());
        iBuffer_->InitializeData(indices);
        OGL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, 0);

        simpleProgram_->BindUniformBlock(perspectiveProjectionUBBName, *app->GetUBOBindingPoints());

        std::vector<std::string> attributeNames;
        SimpleVertex::GatherAttributeNames(attributeNames);

        auto shaderPositions = simpleProgram_->GetAttributeLocations(attributeNames);

        OGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, vBuffer_->GetBuffer());
        drawAttribBinds_.GetVertexAttributes().push_back(simpleProgram_->CreateVertexAttributeArray(vBuffer_->GetBuffer(), iBuffer_->GetBuffer()));
        SimpleVertex::VertexAttributeSetup(drawAttribBinds_.GetVertexAttributes().back(), shaderPositions);
        OGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, 0);

        drawAttribBinds_.GetUniformIds() = simpleProgram_->GetUniformLocations({ "modelMatrix", "color", "pointSize" });
    }

    SimpleMeshRenderer::~SimpleMeshRenderer() = default;

    void SimpleMeshRenderer::DrawCone(const glm::mat4& modelMatrix, const glm::vec4& color) const
    {
        DrawSubmesh(modelMatrix, color, 0);
    }

    void SimpleMeshRenderer::DrawCube(const glm::mat4& modelMatrix, const glm::vec4& color) const
    {
        DrawSubmesh(modelMatrix, color, 1);
    }

    void SimpleMeshRenderer::DrawCylinder(const glm::mat4& modelMatrix, const glm::vec4& color) const
    {
        DrawSubmesh(modelMatrix, color, 2);
    }

    void SimpleMeshRenderer::DrawOctahedron(const glm::mat4& modelMatrix, const glm::vec4& color) const
    {
        DrawSubmesh(modelMatrix, color, 3);
    }

    void SimpleMeshRenderer::DrawSphere(const glm::mat4& modelMatrix, const glm::vec4& color) const
    {
        DrawSubmesh(modelMatrix, color, 4);
    }

    void SimpleMeshRenderer::DrawTorus(const glm::mat4& modelMatrix, const glm::vec4& color) const
    {
        DrawSubmesh(modelMatrix, color, 5);
    }

    void SimpleMeshRenderer::DrawPoint(const glm::mat4& modelMatrix, const glm::vec4& color, float pointSize) const
    {
        DrawSubmesh(modelMatrix, color, 6, pointSize);
    }

    void SimpleMeshRenderer::DrawLine(const glm::mat4& modelMatrix, const glm::vec4& color) const
    {
        DrawSubmesh(modelMatrix, color, 7);
    }

    void SimpleMeshRenderer::DrawSubmesh(const glm::mat4& modelMatrix, const glm::vec4& color, unsigned submeshId, float pointSize) const
    {
        simpleProgram_->UseProgram();
        simpleProgram_->SetUniform(drawAttribBinds_.GetUniformIds()[0], modelMatrix);
        simpleProgram_->SetUniform(drawAttribBinds_.GetUniformIds()[1], color);
        simpleProgram_->SetUniform(drawAttribBinds_.GetUniformIds()[2], pointSize);


        OGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, vBuffer_->GetBuffer());
        drawAttribBinds_.GetVertexAttributes()[0]->EnableVertexAttributeArray();

        auto primitiveType = GL_TRIANGLES;
        if (submeshId == 6) primitiveType = GL_POINTS;
        if (submeshId == 7) primitiveType = GL_LINES;
        OGL_CALL(glDrawElements, primitiveType, submeshInfo_[submeshId].second, GL_UNSIGNED_INT,
            (static_cast<char*> (nullptr)) + (submeshInfo_[submeshId].first * sizeof(unsigned int)));

        drawAttribBinds_.GetVertexAttributes()[0]->DisableVertexAttributeArray();
    }
}
