/**
 * @file   SubMesh.h
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2014.01.13
 *
 * @brief  Contains the sub-mesh class.
 */

#ifndef SUBMESH_H
#define SUBMESH_H

#include "main.h"
#include <core/math/primitives.h>

namespace cgu {

    class Material;
    class Mesh;

    /**
     * A SubMesh is a sub group of geometry in a mesh. It does not have its own
     * vertex information but uses indices to define which vertices of the mesh are used.
     */
    class SubMesh
    {
    public:
        SubMesh() : indexOffset_{ 0 }, numIndices_{ 0 }, material_{ nullptr } { aabb_.minmax[0] = glm::vec3(std::numeric_limits<float>::infinity()); aabb_.minmax[1] = glm::vec3(-std::numeric_limits<float>::infinity()); }
        SubMesh(const Mesh* mesh, const std::string& objectName, unsigned int indexOffset, unsigned int numIndices, Material* material);
        SubMesh(const SubMesh&);
        SubMesh& operator=(const SubMesh&);
        SubMesh(SubMesh&&);
        SubMesh& operator=(SubMesh&&);
        virtual ~SubMesh();

        const std::string& GetName() const { return objectName_; }
        unsigned int GetIndexOffset() const { return indexOffset_; };
        unsigned int GetNumberOfIndices() const { return numIndices_; };
        unsigned int GetNumberOfTriangles() const { return numIndices_ / 3; };
        const cguMath::AABB3<float>& GetLocalAABB() const { return aabb_; }

        void write(std::ofstream& ofs) const;
        void read(std::ifstream& ifs, std::unordered_map<uint64_t, SubMesh*>& meshes, std::unordered_map<uint64_t, Material*>& materials);

    private:
        /** Holds the sub-meshes object name. */
        std::string objectName_;
        /** The index offset the sub-mesh starts. */
        unsigned int indexOffset_;
        /** The number of indices in the sub-mesh. */
        unsigned int numIndices_;
        /** The sub-meshes local AABB. */
        cguMath::AABB3<float> aabb_;
        /** The sub-meshes material. */
        Material* material_;
    };
}

#endif /* SUBMESH_H */
