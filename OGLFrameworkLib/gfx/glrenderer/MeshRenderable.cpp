/**
 * @file   MeshRenderable.cpp
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2015.12.15
 *
 * @brief  Contains the implementation of MeshRenderable.
 */

#include "MeshRenderable.h"
#include "GPUProgram.h"
#include "GLTexture2D.h"
#include "GLTexture.h"

namespace cgu {

    /**
     * Constructor.
     * @param renderMesh the Mesh to use for rendering.
     * @param prog the program used for rendering.
     */
    MeshRenderable::MeshRenderable(const Mesh* renderMesh, GPUProgram* prog) :
        mesh(renderMesh),
        vBuffer(0),
        iBuffer(0),
        drawProgram(prog)
    {
        OGL_CALL(glGenBuffers, 1, &vBuffer);
        OGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, vBuffer);
        OGL_CALL(glBufferData, GL_ARRAY_BUFFER, mesh->faceVertices.size() * sizeof(FaceVertex),
            mesh->faceVertices.data(), GL_STATIC_DRAW);

        OGL_CALL(glGenBuffers, 1, &iBuffer);
        FillIndexBuffer(iBuffer, mesh);

        iBuffers.resize(mesh->subMeshes.size(), 0);
        OGL_CALL(glGenBuffers, static_cast<GLsizei>(iBuffers.size()), iBuffers.data());
        for (unsigned int idx = 0; idx < iBuffers.size(); ++idx) {
            FillIndexBuffer(iBuffers[idx], mesh->subMeshes[idx]);
        }

        OGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, 0);

        FillMeshAttributeBindings(drawProgram, drawAttribBinds);
    }

    /**
     * Destructor.
     */
    MeshRenderable::~MeshRenderable()
    {
        if (vBuffer != 0) {
            OGL_CALL(glDeleteBuffers, 1, &vBuffer);
        }

        if (iBuffer != 0) {
            OGL_CALL(glDeleteBuffers, 1, &iBuffer);
        }

        if (iBuffers.size() > 0) {
            OGL_CALL(glDeleteBuffers, static_cast<GLsizei>(iBuffers.size()), iBuffers.data());
            iBuffers.clear();
        }
        mesh = nullptr;
    }

    /**
     * Copy constructor.
     * @param orig the original object
     */
    MeshRenderable::MeshRenderable(const MeshRenderable& orig) :
        MeshRenderable(orig.mesh, orig.drawProgram)
    {
    }

    /**
     * Move constructor.
     * @param orig the original object
     */
    MeshRenderable::MeshRenderable(MeshRenderable&& orig) :
        mesh(orig.mesh),
        vBuffer(orig.vBuffer),
        iBuffer(orig.iBuffer),
        iBuffers(std::move(orig.iBuffers)),
        drawProgram(orig.drawProgram),
        drawAttribBinds(std::move(orig.drawAttribBinds))
    {
        orig.mesh = nullptr;
        orig.vBuffer = 0;
        orig.iBuffer = 0;
        orig.drawProgram = nullptr;
        iBuffers.clear();
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
            mesh = orig.mesh;
            vBuffer = orig.vBuffer;
            iBuffer = orig.iBuffer;
            iBuffers = std::move(orig.iBuffers);
            drawProgram = orig.drawProgram;
            drawAttribBinds = std::move(orig.drawAttribBinds);
            orig.mesh = nullptr;
            orig.vBuffer = 0;
            orig.iBuffer = 0;
            orig.drawProgram = nullptr;
        }
        return *this;
    }

    void MeshRenderable::FillMeshAttributeBindings(GPUProgram* program, ShaderMeshAttributes& attribBinds) const
    {
        assert(attribBinds.GetUniformIds().size() == 0);
        assert(attribBinds.GetVertexAttributes().size() == 0);
        auto shaderPositions = program->GetAttributeLocations({ "pos", "tex", "normal" });

        OGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, vBuffer);
        attribBinds.GetVertexAttributes().push_back(program->CreateVertexAttributeArray(vBuffer, iBuffer));
        GenerateVertexAttribute(attribBinds.GetVertexAttributes().back(), mesh, shaderPositions);
        for (unsigned int idx = 0; idx < mesh->subMeshes.size(); ++idx) {
            attribBinds.GetVertexAttributes().push_back(program->CreateVertexAttributeArray(vBuffer, iBuffers[idx]));
            GenerateVertexAttribute(attribBinds.GetVertexAttributes().back(), mesh->subMeshes[idx], shaderPositions);
        }
        OGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, 0);

        attribBinds.GetUniformIds() = program->GetUniformLocations({ "diffuseTex", "bumpTex", "bumpMultiplier" });
    }

    void MeshRenderable::Draw() const
    {
        Draw<true>(drawProgram, drawAttribBinds);
    }

    template <bool useMaterials>
    void MeshRenderable::Draw(GPUProgram* program, const ShaderMeshAttributes& attribBinds) const
    {
        program->UseProgram();
        OGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, vBuffer);
        DrawSubMesh<true>(program, attribBinds, attribBinds.GetVertexAttributes()[0], mesh);
        for (unsigned int idx = 0; idx < iBuffers.size(); ++idx) {
            DrawSubMesh<true>(program, attribBinds, attribBinds.GetVertexAttributes()[idx + 1], mesh->subMeshes[idx]);
        }
        OGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, 0);
    }

    void MeshRenderable::FillIndexBuffer(GLuint iBuffer, const SubMesh* subMesh)
    {
        OGL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, iBuffer);
        OGL_CALL(glBufferData, GL_ELEMENT_ARRAY_BUFFER, subMesh->faceIndices.size() * sizeof(unsigned int),
            subMesh->faceIndices.data(), GL_STATIC_DRAW);
    }

    void MeshRenderable::GenerateVertexAttribute(GLVertexAttributeArray* vao, const SubMesh* subMesh,
        const std::vector<BindingLocation>& shaderPositions)
    {
        vao->StartAttributeSetup();
        // pos
        if (shaderPositions[0]->iBinding >= 0) {
            vao->AddVertexAttribute(shaderPositions[0], 3, GL_FLOAT, GL_FALSE, sizeof(FaceVertex), 0);
        }

        if (subMesh->faceHasTexture && shaderPositions[1]->iBinding >= 0) {
            vao->AddVertexAttribute(shaderPositions[1], 2, GL_FLOAT, GL_FALSE,
                sizeof(FaceVertex), sizeof(glm::vec3));
        }
        if (subMesh->faceHasNormal && shaderPositions[2]->iBinding >= 0) {
            vao->AddVertexAttribute(shaderPositions[2], 3, GL_FLOAT, GL_FALSE,
                sizeof(FaceVertex), sizeof(glm::vec3) + sizeof(glm::vec2));
        }
        vao->EndAttributeSetup();
    }

    template<bool useMaterials> void MeshRenderable::DrawSubMesh(GPUProgram* program, const ShaderMeshAttributes& attribBinds,
        const GLVertexAttributeArray* vao, const SubMesh* subMesh) const
    {
        vao->EnableVertexAttributeArray();
        for (const auto& mtlChunk : subMesh->mtlChunks) {
            UseMaterials<useMaterials>(program, attribBinds, mtlChunk);

            GLsizei count = mtlChunk.face_seq_num;
            OGL_CALL(glDrawElements, GL_TRIANGLES, count, GL_UNSIGNED_INT,
                (static_cast<char*> (nullptr)) + (mtlChunk.face_seq_begin * sizeof(unsigned int)));
        }
        vao->DisableVertexAttributeArray();
    }

    template<bool useMaterials> void MeshRenderable::UseMaterials(GPUProgram*, const ShaderMeshAttributes&, const SubMeshMaterialChunk&) const {}

    template<> void MeshRenderable::UseMaterials<true>(GPUProgram* program, const ShaderMeshAttributes& attribBinds,
        const SubMeshMaterialChunk& mtlChunk) const
    {
        if (mtlChunk.material->diffuseTex && attribBinds.GetUniformIds().size() != 0) {
            mtlChunk.material->diffuseTex->GetTexture()->ActivateTexture(GL_TEXTURE0);
            program->SetUniform(attribBinds.GetUniformIds()[0], 0);
        }
        if (mtlChunk.material->bumpTex && attribBinds.GetUniformIds().size() >= 2) {
            mtlChunk.material->bumpTex->GetTexture()->ActivateTexture(GL_TEXTURE1);
            program->SetUniform(attribBinds.GetUniformIds()[1], 1);
            program->SetUniform(attribBinds.GetUniformIds()[2], mtlChunk.material->bumpMultiplier);
        }
    }

    /**
     *  Constructor.
     *  @param renderMesh the mesh to render.
     *  @param prog the GPU program for rendering.
     *  @param shadowProg the GPU program for shadow map rendering.
     */
    MeshRenderableShadowing::MeshRenderableShadowing(const Mesh* renderMesh, GPUProgram* prog, GPUProgram* shadowProg) :
        MeshRenderable(renderMesh, prog),
        shadowProgram(shadowProg)

    {
        FillMeshAttributeBindings(shadowProgram, shadowAttribBinds);
    }

    /** Helper constructor for implementing the copy constructor. */
    MeshRenderableShadowing::MeshRenderableShadowing(const MeshRenderable& rhs, GPUProgram* shadowProg) :
        MeshRenderable(rhs),
        shadowProgram(shadowProg)
    {
        FillMeshAttributeBindings(shadowProgram, shadowAttribBinds);
    }

    /** Copy constructor. */
    MeshRenderableShadowing::MeshRenderableShadowing(const MeshRenderableShadowing& rhs) = default;
    /** Copy assignment operator. */
    MeshRenderableShadowing& MeshRenderableShadowing::operator=(const MeshRenderableShadowing& rhs) = default;

    /** Move constructor. */
    MeshRenderableShadowing::MeshRenderableShadowing(MeshRenderableShadowing&& rhs) :
        MeshRenderable(std::move(rhs)),
        shadowProgram(rhs.shadowProgram),
        shadowAttribBinds(std::move(rhs.shadowAttribBinds))
    {
        rhs.shadowProgram = nullptr;
    }

    /** Move assignment operator. */
    MeshRenderableShadowing& MeshRenderableShadowing::operator=(MeshRenderableShadowing&& rhs)
    {
        if (this != &rhs) {
            this->~MeshRenderableShadowing();
            MeshRenderable::operator =(std::move(rhs));
            shadowProgram = std::move(rhs.shadowProgram);
            shadowAttribBinds = std::move(rhs.shadowAttribBinds);
        }
        return *this;
    }

    MeshRenderableShadowing::~MeshRenderableShadowing() = default;

    void MeshRenderableShadowing::DrawShadow() const
    {
        Draw<false>(shadowProgram, shadowAttribBinds);
    }
}
