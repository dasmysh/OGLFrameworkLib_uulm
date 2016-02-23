/**
 * @file   MeshRenderable.h
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2014.01.19
 *
 * @brief  Contains the definition of MeshRenderable.
 */

#ifndef MESHRENDERABLE_H
#define MESHRENDERABLE_H

#include "gfx/mesh/Mesh.h"
#include "main.h"
#include "GPUProgram.h"
#include "gfx/glrenderer/ShaderMeshAttributes.h"

namespace cgu {

    /**
     * @brief  Renderable implementation for triangle meshes.
     *
     * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
     * @date   2015.12.15
     */
    class MeshRenderable
    {
    public:
        template<class VTX> static std::unique_ptr<MeshRenderable> create(const Mesh* renderMesh, GPUProgram* program);
        virtual ~MeshRenderable();
        MeshRenderable(const MeshRenderable&);
        MeshRenderable& operator=(const MeshRenderable&);
        MeshRenderable(MeshRenderable&&);
        MeshRenderable& operator=(MeshRenderable&&);

        void Draw(const glm::mat4& modelMatrix) const;

    protected:
        MeshRenderable(const Mesh* renderMesh, GPUProgram* program);
        template<class VTX> void CreateVertexBuffer();
        template<bool useMaterials> void Draw(const glm::mat4& modelMatrix, GPUProgram* program, const ShaderMeshAttributes& attribBinds) const;
        template<bool useMaterials> void DrawNode(const glm::mat4& modelMatrix, const SceneMeshNode* node, GPUProgram* program, const ShaderMeshAttributes& attribBinds) const;
        template<class VTX> void FillMeshAttributeBindings(GPUProgram* program, ShaderMeshAttributes& attribBinds) const;

    private:
        /** Holds the mesh to render. */
        const Mesh* mesh_;
        /** Holds the vertex buffer object name. */
        BufferRAII vBuffer_;
        /** Holds the index buffer object name of the mesh base. */
        BufferRAII iBuffer_;
        /** Holds the rendering GPU program for drawing. */
        GPUProgram* drawProgram_;
        /** Holds the shader attribute bindings for the shader. */
        ShaderMeshAttributes drawAttribBinds_;

        template<class VTX> static void GenerateVertexAttribute(GLVertexAttributeArray* vao, const std::vector<BindingLocation>& shaderPositions);
        template<bool useMaterials> void DrawSubMesh(const glm::mat4& modelMatrix, GPUProgram* program,
            const ShaderMeshAttributes& attribBinds, const SubMesh* subMesh) const;
        template<bool useMaterials> void UseMaterials(GPUProgram* program, const ShaderMeshAttributes& attribBinds,
            const SubMesh* subMesh) const;
    };

    /**
     *  Renderable for meshes that cast shadows into a shadow map.
     */
    class MeshRenderableShadowing : public MeshRenderable
    {
    public:
        template<class VTX> static std::unique_ptr<MeshRenderableShadowing> create(const Mesh* renderMesh, GPUProgram* program, GPUProgram* shadowProgram);
        template<class VTX> static std::unique_ptr<MeshRenderableShadowing> create(const MeshRenderable& rhs, GPUProgram* shadowProgram);
        MeshRenderableShadowing(const MeshRenderableShadowing&);
        MeshRenderableShadowing& operator=(const MeshRenderableShadowing&);
        MeshRenderableShadowing(MeshRenderableShadowing&&);
        MeshRenderableShadowing& operator=(MeshRenderableShadowing&&);
        virtual ~MeshRenderableShadowing();

        void DrawShadow(const glm::mat4& modelMatrix) const;

    protected:
        MeshRenderableShadowing(const Mesh* renderMesh, GPUProgram* program, GPUProgram* shadowProgram);
        MeshRenderableShadowing(const MeshRenderable& rhs, GPUProgram* shadowProgram);
        template<class VTX> void CreateVertexBuffer();

    private:
        /** Holds the rendering GPU program for shadow map drawing. */
        GPUProgram* shadowProgram_;
        /** Holds the shader attribute bindings for the shadow shader. */
        ShaderMeshAttributes shadowAttribBinds_;
    };

    template <class VTX>
    std::unique_ptr<MeshRenderable> MeshRenderable::create(const Mesh* renderMesh, GPUProgram* program)
    {
        std::unique_ptr<MeshRenderable> result{ new MeshRenderable(renderMesh, program) };
        result->CreateVertexBuffer<VTX>();
        return std::move(result);
    }

    template <class VTX>
    void MeshRenderable::CreateVertexBuffer()
    {
        OGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, vBuffer_);
        std::vector<VTX> vertices;
        mesh_->GetVertices(vertices);
        OGL_CALL(glBufferData, GL_ARRAY_BUFFER, vertices.size() * sizeof(VTX), vertices.data(), GL_STATIC_DRAW);
        OGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, 0);

        FillMeshAttributeBindings<VTX>(drawProgram_, drawAttribBinds_);
    }

    template<class VTX> void MeshRenderable::FillMeshAttributeBindings(GPUProgram* program, ShaderMeshAttributes& attribBinds) const
    {
        assert(attribBinds.GetUniformIds().size() == 0);
        assert(attribBinds.GetVertexAttributes().size() == 0);
        std::vector<std::string> attributeNames;
        VTX::GatherAttributeNames(attributeNames);
        /*{ { "position", "normal", "tangent", "binormal" } };
        std::stringstream attributeNameStr;
        for (auto i = 0; i < VTX::NUM_TEXTURECOORDS; ++i) { attributeNameStr.clear(); attributeNameStr << "tex[" << i << "]"; attributeNames.push_back(attributeNameStr.str()); }
        for (auto i = 0; i < VTX::NUM_COLORS; ++i) { attributeNameStr.clear(); attributeNameStr << "color[" << i << "]"; attributeNames.push_back(attributeNameStr.str()); }*/

        auto shaderPositions = program->GetAttributeLocations(attributeNames);

        OGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, vBuffer_);
        attribBinds.GetVertexAttributes().push_back(program->CreateVertexAttributeArray(vBuffer_, iBuffer_));
        GenerateVertexAttribute<VTX>(attribBinds.GetVertexAttributes().back(), shaderPositions);
        /*for (unsigned int idx = 0; idx < mesh->subMeshes.size(); ++idx) {
        attribBinds.GetVertexAttributes().push_back(program->CreateVertexAttributeArray(vBuffer, iBuffers[idx]));
        GenerateVertexAttribute(attribBinds.GetVertexAttributes().back(), mesh->subMeshes[idx], shaderPositions);
        }*/
        OGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, 0);

        attribBinds.GetUniformIds() = program->GetUniformLocations({ "modelMatrix", "diffuseTex", "bumpTex", "bumpMultiplier" });
    }

    template<class VTX> void MeshRenderable::GenerateVertexAttribute(GLVertexAttributeArray* vao, const std::vector<BindingLocation>& shaderPositions)
    {
        VTX::VertexAttributeSetup(vao, shaderPositions);
        /*vao->StartAttributeSetup();
        auto cPos = 0;
        if (shaderPositions[cPos]->iBinding >= 0) {
            vao->AddVertexAttribute(shaderPositions[cPos++], VTX::POSITION_DIMENSION, GL_FLOAT, GL_FALSE, sizeof(VTX), offsetof(VTX, pos));
        }
        if (shaderPositions[cPos]->iBinding >= 0) {
            vao->AddVertexAttribute(shaderPositions[cPos++], 3, GL_FLOAT, GL_FALSE, sizeof(VTX), offsetof(VTX, normal));
        }
        if (shaderPositions[cPos]->iBinding >= 0) {
            vao->AddVertexAttribute(shaderPositions[cPos++], 3, GL_FLOAT, GL_FALSE, sizeof(VTX), offsetof(VTX, tangent));
        }
        if (shaderPositions[cPos]->iBinding >= 0) {
            vao->AddVertexAttribute(shaderPositions[cPos++], 3, GL_FLOAT, GL_FALSE, sizeof(VTX), offsetof(VTX, binormal));
        }
        for (auto i = 0; i < VTX::NUM_TEXTURECOORDS; ++i) {
            if (shaderPositions[cPos]->iBinding >= 0)
                vao->AddVertexAttribute(shaderPositions[cPos++], VTX::TEXCOORD_DIMENSION, GL_FLOAT, GL_FALSE, sizeof(VTX), offsetof(VTX, tex[i]));
        }
        for (auto i = 0; i < VTX::NUM_COLORS; ++i) {
            if (shaderPositions[cPos]->iBinding >= 0)
                vao->AddVertexAttribute(shaderPositions[cPos++], 4, GL_FLOAT, GL_FALSE, sizeof(VTX), offsetof(VTX, color[i]));
        }
        vao->EndAttributeSetup();*/
    }

    template <class VTX>
    std::unique_ptr<MeshRenderableShadowing> MeshRenderableShadowing::create(const Mesh* renderMesh, GPUProgram* program, GPUProgram* shadowProgram)
    {
        std::unique_ptr<MeshRenderableShadowing> result{ new MeshRenderableShadowing(renderMesh, program, shadowProgram) };
        result->CreateVertexBuffer<VTX>();
        return std::move(result);
    }

    template <class VTX>
    std::unique_ptr<MeshRenderableShadowing> MeshRenderableShadowing::create(const MeshRenderable& rhs, GPUProgram* shadowProgram)
    {
        std::unique_ptr<MeshRenderableShadowing> result{ new MeshRenderableShadowing(rhs, shadowProgram) };
        result->CreateVertexBuffer<VTX>();
        return std::move(result);
    }

    template <class VTX>
    void MeshRenderableShadowing::CreateVertexBuffer()
    {
        MeshRenderable::CreateVertexBuffer<VTX>();
        FillMeshAttributeBindings<VTX>(shadowProgram_, shadowAttribBinds_);
    }
}

#endif /* MESHRENDERABLE_H */
