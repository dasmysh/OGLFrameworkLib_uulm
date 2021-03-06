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

    class GLBuffer;

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
        template<class VTX> static std::unique_ptr<MeshRenderable> create(const Mesh* renderMesh, const GLBuffer* iBuffer, GPUProgram* program);
        template<class VTX> static std::unique_ptr<MeshRenderable> create(const Mesh* renderMesh, const GLBuffer* vBuffer, const GLBuffer* iBuffer, GPUProgram* program);
        virtual ~MeshRenderable();
        MeshRenderable(const MeshRenderable&);
        MeshRenderable& operator=(const MeshRenderable&);
        MeshRenderable(MeshRenderable&&);
        MeshRenderable& operator=(MeshRenderable&&);

        void Draw(const glm::mat4& modelMatrix, bool overrideBump = false) const;
        void DrawPart(const glm::mat4& modelMatrix, unsigned int start, unsigned int count, GLenum mode) const;
        // void BindAsShaderBuffer(GLuint bindingPoint) const;

    protected:
        MeshRenderable(const Mesh* renderMesh, const GLBuffer* vBuffer, GPUProgram* program);
        MeshRenderable(const Mesh* renderMesh, const GLBuffer* vBuffer, const GLBuffer* iBuffer, GPUProgram* program);
        template<class VTX> void CreateVertexAttributeBuffer();
        template<bool useMaterials> void Draw(const glm::mat4& modelMatrix, GPUProgram* program, const ShaderMeshAttributes& attribBinds, bool overrideBump = false) const;
        template<bool useMaterials> void DrawNode(const glm::mat4& modelMatrix, const SceneMeshNode* node, GPUProgram* program, const ShaderMeshAttributes& attribBinds, bool overrideBump = false) const;
        template<class VTX> void FillMeshAttributeBindings(GPUProgram* program, ShaderMeshAttributes& attribBinds) const;

    private:
        /** Holds the mesh to render. */
        const Mesh* mesh_;
        /** Holds the vertex buffer. */
        const GLBuffer* vBuffer_;
        /** Holds the index buffer of the mesh base. */
        const GLBuffer* iBuffer_;
        /** Holds the vertex buffer object name. */
        // BufferRAII vBuffer_;
        /** Holds the index buffer object name of the mesh base. */
        // BufferRAII iBuffer_;
        /** Holds the rendering GPU program for drawing. */
        GPUProgram* drawProgram_;
        /** Holds the shader attribute bindings for the shader. */
        ShaderMeshAttributes drawAttribBinds_;

        template<class VTX> static void GenerateVertexAttribute(GLVertexAttributeArray* vao, const std::vector<BindingLocation>& shaderPositions);
        template<bool useMaterials> void DrawSubMesh(const glm::mat4& modelMatrix, GPUProgram* program,
            const ShaderMeshAttributes& attribBinds, const SubMesh* subMesh, bool overrideBump = false) const;
        template<bool useMaterials> void UseMaterials(GPUProgram* program, const ShaderMeshAttributes& attribBinds,
            const SubMesh* subMesh, bool overrideBump = false) const;
    };

    /**
     *  Renderable for meshes that cast shadows into a shadow map.
     */
    class MeshRenderableShadowing : public MeshRenderable
    {
    public:
        template<class VTX> static std::unique_ptr<MeshRenderableShadowing> create(const Mesh* renderMesh, const GLBuffer* vBuffer, GPUProgram* program, GPUProgram* shadowProgram);
        template<class VTX> static std::unique_ptr<MeshRenderableShadowing> create(const Mesh* renderMesh, GPUProgram* program, GPUProgram* shadowProgram);
        template<class VTX> static std::unique_ptr<MeshRenderableShadowing> create(const MeshRenderable& rhs, GPUProgram* shadowProgram);
        MeshRenderableShadowing(const MeshRenderableShadowing&);
        MeshRenderableShadowing& operator=(const MeshRenderableShadowing&);
        MeshRenderableShadowing(MeshRenderableShadowing&&);
        MeshRenderableShadowing& operator=(MeshRenderableShadowing&&);
        virtual ~MeshRenderableShadowing();

        void DrawShadow(const glm::mat4& modelMatrix) const;

    protected:
        MeshRenderableShadowing(const Mesh* renderMesh, const GLBuffer* vBuffer, GPUProgram* program, GPUProgram* shadowProgram);
        MeshRenderableShadowing(const MeshRenderable& rhs, GPUProgram* shadowProgram);
        template<class VTX> void CreateVertexAttributeBuffer();

    private:
        /** Holds the rendering GPU program for shadow map drawing. */
        GPUProgram* shadowProgram_;
        /** Holds the shader attribute bindings for the shadow shader. */
        ShaderMeshAttributes shadowAttribBinds_;
    };

    template <class VTX>
    std::unique_ptr<MeshRenderable> MeshRenderable::create(const Mesh* renderMesh, GPUProgram* program)
    {
        std::unique_ptr<MeshRenderable> result{ new MeshRenderable(renderMesh, renderMesh->GetVertexBuffer<VTX>(), program) };
        result->CreateVertexAttributeBuffer<VTX>();
        return std::move(result);
    }

    template <class VTX>
    std::unique_ptr<MeshRenderable> MeshRenderable::create(const Mesh* renderMesh, const GLBuffer* iBuffer, GPUProgram* program)
    {
        std::unique_ptr<MeshRenderable> result{ new MeshRenderable(renderMesh, renderMesh->GetVertexBuffer<VTX>(), iBuffer, program) };
        result->CreateVertexAttributeBuffer<VTX>();
        return std::move(result);
    }

    template <class VTX>
    std::unique_ptr<MeshRenderable> MeshRenderable::create(const Mesh* renderMesh, const GLBuffer* vBuffer, const GLBuffer* iBuffer, GPUProgram* program)
    {
        std::unique_ptr<MeshRenderable> result{ new MeshRenderable(renderMesh, vBuffer, iBuffer, program) };
        result->CreateVertexAttributeBuffer<VTX>();
        return std::move(result);
    }

    template <class VTX>
    void MeshRenderable::CreateVertexAttributeBuffer()
    {
        FillMeshAttributeBindings<VTX>(drawProgram_, drawAttribBinds_);
    }

    template<class VTX> void MeshRenderable::FillMeshAttributeBindings(GPUProgram* program, ShaderMeshAttributes& attribBinds) const
    {
        assert(attribBinds.GetUniformIds().size() == 0);
        assert(attribBinds.GetVertexAttributes().size() == 0);
        std::vector<std::string> attributeNames;
        VTX::GatherAttributeNames(attributeNames);

        auto shaderPositions = program->GetAttributeLocations(attributeNames);

        OGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, vBuffer_->GetBuffer());
        attribBinds.GetVertexAttributes().push_back(program->CreateVertexAttributeArray(vBuffer_->GetBuffer(), iBuffer_->GetBuffer()));
        GenerateVertexAttribute<VTX>(attribBinds.GetVertexAttributes().back(), shaderPositions);
        OGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, 0);

        attribBinds.GetUniformIds() = program->GetUniformLocations({ "modelMatrix", "normalMatrix", "diffuseTex", "bumpTex", "bumpMultiplier" });
    }

    template<class VTX> void MeshRenderable::GenerateVertexAttribute(GLVertexAttributeArray* vao, const std::vector<BindingLocation>& shaderPositions)
    {
        VTX::VertexAttributeSetup(vao, shaderPositions);
    }

    template <class VTX>
    std::unique_ptr<MeshRenderableShadowing> MeshRenderableShadowing::create(const Mesh* renderMesh, const GLBuffer* vBuffer, GPUProgram* program, GPUProgram* shadowProgram)
    {
        std::unique_ptr<MeshRenderableShadowing> result{ new MeshRenderableShadowing(renderMesh, vBuffer, program, shadowProgram) };
        result->CreateVertexAttributeBuffer<VTX>();
        return std::move(result);
    }

    template <class VTX>
    std::unique_ptr<MeshRenderableShadowing> MeshRenderableShadowing::create(const Mesh* renderMesh, GPUProgram* program, GPUProgram* shadowProgram)
    {
        return std::move(MeshRenderableShadowing::create<VTX>(renderMesh, renderMesh->GetVertexBuffer<VTX>(), program, shadowProgram));
    }

    template <class VTX>
    std::unique_ptr<MeshRenderableShadowing> MeshRenderableShadowing::create(const MeshRenderable& rhs, GPUProgram* shadowProgram)
    {
        std::unique_ptr<MeshRenderableShadowing> result{ new MeshRenderableShadowing(rhs, shadowProgram) };
        result->CreateVertexAttributeBuffer<VTX>();
        return std::move(result);
    }

    template <class VTX>
    void MeshRenderableShadowing::CreateVertexAttributeBuffer()
    {
        MeshRenderable::CreateVertexAttributeBuffer<VTX>();
        FillMeshAttributeBindings<VTX>(shadowProgram_, shadowAttribBinds_);
    }
}

#endif /* MESHRENDERABLE_H */
