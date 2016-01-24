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
        MeshRenderable(const Mesh* renderMesh, GPUProgram* program);
        virtual ~MeshRenderable();
        MeshRenderable(const MeshRenderable&);
        MeshRenderable& operator=(const MeshRenderable&);
        MeshRenderable(MeshRenderable&&);
        MeshRenderable& operator=(MeshRenderable&&);

        void Draw() const;

    protected:
        template<bool useMaterials> void Draw(GPUProgram* program, const ShaderMeshAttributes& attribBinds) const;
        void FillMeshAttributeBindings(GPUProgram* program, ShaderMeshAttributes& attribBinds) const;

    private:
        /** Holds the mesh to render. */
        const Mesh* mesh;
        /** Holds the vertex buffer object name. */
        BufferRAII vBuffer;
        /** Holds the index buffer object name of the mesh base. */
        BufferRAII iBuffer;
        /** Holds the index buffer object names. */
        std::vector<BufferRAII> iBuffers;
        /** Holds the rendering GPU program for drawing. */
        GPUProgram* drawProgram;
        /** Holds the shader attribute bindings for the shader. */
        ShaderMeshAttributes drawAttribBinds;

        static void FillIndexBuffer(GLuint iBuffer, const SubMesh* subMesh);
        static void GenerateVertexAttribute(GLVertexAttributeArray* vao, const SubMesh* subMesh,
            const std::vector<BindingLocation>& shaderPositions);
        template<bool useMaterials> void DrawSubMesh(GPUProgram* program, const ShaderMeshAttributes& attribBinds,
            const GLVertexAttributeArray* vao, const SubMesh* subMesh) const;
        template<bool useMaterials> void UseMaterials(GPUProgram* program, const ShaderMeshAttributes& attribBinds,
            const SubMeshMaterialChunk&) const;
    };

    /**
     *  Renderable for meshes that cast shadows into a shadow map.
     */
    class MeshRenderableShadowing : public MeshRenderable
    {
    public:
        MeshRenderableShadowing(const Mesh* renderMesh, GPUProgram* program, GPUProgram* shadowProgram);
        MeshRenderableShadowing(const MeshRenderable& rhs, GPUProgram* shadowProgram);
        MeshRenderableShadowing(const MeshRenderableShadowing&);
        MeshRenderableShadowing& operator=(const MeshRenderableShadowing&);
        MeshRenderableShadowing(MeshRenderableShadowing&&);
        MeshRenderableShadowing& operator=(MeshRenderableShadowing&&);
        virtual ~MeshRenderableShadowing();

        void DrawShadow() const;

    private:
        /** Holds the rendering GPU program for shadow map drawing. */
        GPUProgram* shadowProgram;
        /** Holds the shader attribute bindings for the shadow shader. */
        ShaderMeshAttributes shadowAttribBinds;
    };
}

#endif /* MESHRENDERABLE_H */
