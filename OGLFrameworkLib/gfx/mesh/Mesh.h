/**
 * @file   Mesh.h
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2014.01.13
 *
 * @brief  Contains the definition of the Mesh class.
 */

#ifndef MESH_H
#define MESH_H

#include "main.h"

struct aiNode;

namespace cgu {

    class TextureManager;
    class SubMesh;
    class SceneMeshNode;

    /**
     * @brief  Base class for all meshes.
     *
     * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
     * @date   2014.01.13
     */
    class Mesh
    {
    public:
        Mesh();
        Mesh(const Mesh&);
        Mesh& operator=(const Mesh&);
        Mesh(Mesh&&);
        Mesh& operator=(Mesh&&);
        virtual ~Mesh();

        unsigned int GetNumSubmeshes() const { return static_cast<unsigned int>(subMeshes_.size()); }
        const SubMesh* GetSubMesh(unsigned int id) const { return subMeshes_[id].get(); }
        const std::vector<glm::vec3>& GetVertices() const { return vertices_; }
        const std::vector<unsigned int>& GetIndices() const { return indices_; }
        const glm::mat4& GetRootTransform() const { return rootTransform_; }
        const SceneMeshNode* GetRootNode() const { return rootNode_.get(); }

    protected:
        std::vector<glm::vec3>& GetVertices() { return vertices_; }
        std::vector<glm::vec3>& GetNormals() { return normals_; }
        const std::vector<glm::vec3>& GetNormals() const { return normals_; }
        std::vector<std::vector<glm::vec3>>& GetTexCoords() { return texCoords_; }
        const std::vector<std::vector<glm::vec3>>& GetTexCoords() const { return texCoords_; }
        std::vector<glm::vec3>& GetTangents() { return tangents_; }
        const std::vector<glm::vec3>& GetTangents() const { return tangents_; }
        std::vector<glm::vec3>& GetBinormals() { return binormals_; }
        const std::vector<glm::vec3>& GetBinormals() const { return binormals_; }
        std::vector<std::vector<glm::vec4>>& GetColors() { return colors_; }
        const std::vector<std::vector<glm::vec4>>& GetColors() const { return colors_; }
        std::vector<unsigned int>& GetIndices() { return indices_; }

        void ReserveMesh(unsigned int maxUVChannels, unsigned int maxColorChannels, unsigned int numVertices, unsigned int numIndices, unsigned int numMaterials);
        Material* GetMaterial(unsigned int id) { return materials_[id].get(); }
        void AddSubMesh(const std::string& name, unsigned int idxOffset, unsigned int numIndices, Material* material);

        void CreateSceneNodes(aiNode* rootNode);

        void write(std::ofstream& ofs) const;
        void read(std::ifstream& ifs, TextureManager& texMan);

    private:
        /** Holds all the single points used by the mesh (and its sub-meshes) as points or in vertices. */
        std::vector<glm::vec3> vertices_;
        /** Holds all the single normals used by the mesh (and its sub-meshes). */
        std::vector<glm::vec3> normals_;
        /** Holds all the single texture coordinates used by the mesh (and its sub-meshes). */
        std::vector<std::vector<glm::vec3>> texCoords_;
        /** Holds all the single tangents used by the mesh (and its sub-meshes). */
        std::vector<glm::vec3> tangents_;
        /** Holds all the single binormals used by the mesh (and its sub-meshes). */
        std::vector<glm::vec3> binormals_;
        /** Holds all the single colors used by the mesh (and its sub-meshes). */
        std::vector<std::vector<glm::vec4>> colors_;
        /** Holds all the indices used by the sub-meshes. */
        std::vector<unsigned int> indices_;

        /** The root transformation for the meshes. */
        glm::mat4 rootTransform_;
        /** The root scene node. */
        std::unique_ptr<SceneMeshNode> rootNode_;

        /** The meshes materials. */
        std::vector<std::unique_ptr<Material>> materials_;

        /** Holds all the meshes sub-meshes (as an array for fast iteration during rendering). */
        std::vector<std::unique_ptr<SubMesh>> subMeshes_;
    };
}

#endif /* MESH_H */
