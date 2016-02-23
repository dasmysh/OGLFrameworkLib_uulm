/**
 * @file   MeshRenderable.cpp
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2015.12.15
 *
 * @brief  Contains the implementation of MeshRenderable.
 */

#include "MeshRenderable.h"
#include "GPUProgram.h"
#include "gfx/mesh/SceneMeshNode.h"
#include "gfx/mesh/SubMesh.h"
#include "gfx/Material.h"
#include "GLTexture2D.h"
#include "GLTexture.h"

namespace cgu {

    /**
     * Constructor.
     * @param renderMesh the Mesh to use for rendering.
     * @param prog the program used for rendering.
     */
    MeshRenderable::MeshRenderable(const Mesh* renderMesh, GPUProgram* program) : mesh_(renderMesh), drawProgram_(program)
    {
        OGL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, iBuffer_);
        OGL_CALL(glBufferData, GL_ELEMENT_ARRAY_BUFFER, mesh_->GetIndices().size() * sizeof(unsigned int),
            mesh_->GetIndices().data(), GL_STATIC_DRAW);
        OGL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    /**
     * Destructor.
     */
    MeshRenderable::~MeshRenderable() = default;

    /**
     * Copy constructor.
     * @param orig the original object
     */
    MeshRenderable::MeshRenderable(const MeshRenderable& orig) :
        mesh_(orig.mesh_),
        drawProgram_(orig.drawProgram_),
        drawAttribBinds_(orig.drawAttribBinds_)
    {
        auto bufferSize = 0;
        OGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, orig.vBuffer_);
        OGL_CALL(glGetBufferParameteriv, GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);

        OGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, vBuffer_);
        OGL_CALL(glBufferData, GL_ARRAY_BUFFER, bufferSize, 0, GL_STATIC_COPY);

        OGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, orig.vBuffer_);
        OGL_CALL(glBindBuffer, GL_COPY_WRITE_BUFFER, vBuffer_);
        OGL_CALL(glCopyBufferSubData, GL_ARRAY_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, bufferSize);
        OGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, 0);
        OGL_CALL(glBindBuffer, GL_COPY_WRITE_BUFFER, 0);

        OGL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, iBuffer_);
        OGL_CALL(glBufferData, GL_ELEMENT_ARRAY_BUFFER, mesh_->GetIndices().size() * sizeof(unsigned int),
            mesh_->GetIndices().data(), GL_STATIC_DRAW);
    }

    /**
     * Move constructor.
     * @param orig the original object
     */
    MeshRenderable::MeshRenderable(MeshRenderable&& orig) :
        mesh_(orig.mesh_),
        vBuffer_(std::move(orig.vBuffer_)),
        iBuffer_(std::move(orig.iBuffer_)),
        drawProgram_(orig.drawProgram_),
        drawAttribBinds_(std::move(orig.drawAttribBinds_))
    {
        orig.mesh_ = nullptr;
        orig.drawProgram_ = nullptr;
    }

    /**
     * Copy assignment operator.
     * @param orig the original object
     */
    MeshRenderable& MeshRenderable::operator=(const MeshRenderable& orig)
    {
        if (this != &orig) {
            MeshRenderable tmp{ orig };
            std::swap(*this, tmp);
        }
        return *this;
    }

    /**
     * Move assignment operator.
     * @param orig the original object
     */
    MeshRenderable& MeshRenderable::operator=(MeshRenderable&& orig)
    {
        if (this != &orig) {
            this->~MeshRenderable();
            mesh_ = orig.mesh_;
            vBuffer_ = std::move(orig.vBuffer_);
            iBuffer_ = std::move(orig.iBuffer_);
            drawProgram_ = orig.drawProgram_;
            drawAttribBinds_ = std::move(orig.drawAttribBinds_);
            orig.mesh_ = nullptr;
            orig.drawProgram_ = nullptr;
        }
        return *this;
    }

    void MeshRenderable::Draw(const glm::mat4& modelMatrix) const
    {
        Draw<true>(modelMatrix, drawProgram_, drawAttribBinds_);
    }

    template <bool useMaterials>
    void MeshRenderable::Draw(const glm::mat4& modelMatrix, GPUProgram* program, const ShaderMeshAttributes& attribBinds) const
    {
        program->UseProgram();
        OGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, vBuffer_);
        attribBinds.GetVertexAttributes()[0]->EnableVertexAttributeArray();
        DrawNode<useMaterials>(mesh_->GetRootTransform() * modelMatrix, mesh_->GetRootNode(), program, attribBinds);
        attribBinds.GetVertexAttributes()[0]->DisableVertexAttributeArray();
        OGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, 0);
    }

    template <bool useMaterials>
    void MeshRenderable::DrawNode(const glm::mat4& modelMatrix, const SceneMeshNode* node, GPUProgram* program, const ShaderMeshAttributes& attribBinds) const
    {
        auto localMatrix = node->GetLocalTransform() * modelMatrix;
        for (unsigned int i = 0; i < node->GetNumMeshes(); ++i) DrawSubMesh<useMaterials>(localMatrix, program, attribBinds, node->GetMesh(i));
        for (unsigned int i = 0; i < node->GetNumNodes(); ++i) DrawNode<useMaterials>(localMatrix, node->GetChild(i), program, attribBinds);
    }

    template<bool useMaterials> void MeshRenderable::DrawSubMesh(const glm::mat4& modelMatrix, GPUProgram* program,
        const ShaderMeshAttributes& attribBinds, const SubMesh* subMesh) const
    {
        program->SetUniform(attribBinds.GetUniformIds()[0], modelMatrix);
        UseMaterials<useMaterials>(program, attribBinds, subMesh);
        OGL_CALL(glDrawElements, GL_TRIANGLES, subMesh->GetNumberOfTriangles(), GL_UNSIGNED_INT,
            (static_cast<char*> (nullptr)) + (subMesh->GetIndexOffset() * sizeof(unsigned int)));
    }

    template<bool useMaterials> void MeshRenderable::UseMaterials(GPUProgram*, const ShaderMeshAttributes&, const SubMesh*) const {}

    template<> void MeshRenderable::UseMaterials<true>(GPUProgram* program, const ShaderMeshAttributes& attribBinds,
        const SubMesh* subMesh) const
    {
        if (subMesh->GetMaterial()->diffuseTex && attribBinds.GetUniformIds().size() != 0) {
            subMesh->GetMaterial()->diffuseTex->GetTexture()->ActivateTexture(GL_TEXTURE0);
            program->SetUniform(attribBinds.GetUniformIds()[1], 0);
        }
        if (subMesh->GetMaterial()->bumpTex && attribBinds.GetUniformIds().size() >= 2) {
            subMesh->GetMaterial()->bumpTex->GetTexture()->ActivateTexture(GL_TEXTURE1);
            program->SetUniform(attribBinds.GetUniformIds()[2], 1);
            program->SetUniform(attribBinds.GetUniformIds()[3], subMesh->GetMaterial()->bumpMultiplier);
        }
    }

    /** Copy constructor. */
    MeshRenderableShadowing::MeshRenderableShadowing(const MeshRenderableShadowing& rhs) = default;
    /** Copy assignment operator. */
    MeshRenderableShadowing& MeshRenderableShadowing::operator=(const MeshRenderableShadowing& rhs) = default;

    /** Move constructor. */
    MeshRenderableShadowing::MeshRenderableShadowing(MeshRenderableShadowing&& rhs) :
        MeshRenderable(std::move(rhs)),
        shadowProgram_(rhs.shadowProgram_),
        shadowAttribBinds_(std::move(rhs.shadowAttribBinds_))
    {
        rhs.shadowProgram_ = nullptr;
    }

    /** Move assignment operator. */
    MeshRenderableShadowing& MeshRenderableShadowing::operator=(MeshRenderableShadowing&& rhs)
    {
        if (this != &rhs) {
            this->~MeshRenderableShadowing();
            MeshRenderable::operator =(std::move(rhs));
            shadowProgram_ = std::move(rhs.shadowProgram_);
            shadowAttribBinds_ = std::move(rhs.shadowAttribBinds_);
        }
        return *this;
    }

    MeshRenderableShadowing::~MeshRenderableShadowing() = default;

    void MeshRenderableShadowing::DrawShadow(const glm::mat4& modelMatrix) const
    {
        Draw<false>(modelMatrix, shadowProgram_, shadowAttribBinds_);
    }

    /**
     *  Constructor.
     *  @param renderMesh the mesh to render.
     *  @param program the GPU program for rendering.
     *  @param shadowProgram the GPU program for shadow map rendering.
     */
    MeshRenderableShadowing::MeshRenderableShadowing(const Mesh* renderMesh, GPUProgram* program, GPUProgram* shadowProgram) :
        MeshRenderable(renderMesh, program),
        shadowProgram_(shadowProgram)
    {
    }

    /** Helper constructor for implementing the copy constructor. */
    MeshRenderableShadowing::MeshRenderableShadowing(const MeshRenderable& rhs, GPUProgram* shadowProgram) :
        MeshRenderable(rhs),
        shadowProgram_(shadowProgram)
    {
    }
}
