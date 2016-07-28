/**
 * @file   ConnectivityMesh.h
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2016.02.18
 *
 * @brief  Contains the definition of a class defining connectivity information in meshes.
 */

#ifndef CONNECTIVITYMESH_H
#define CONNECTIVITYMESH_H

#include "main.h"

namespace cgu {

    class Mesh;
    class ConnectivitySubMesh;

    namespace impl {
        class ConnectivityMeshImpl;
    }

    /** Contains vertex connectivity information. */
    struct MeshConnectVertex
    {
        MeshConnectVertex() : idx{ 0 }, locOnlyIdx{ 0 }, chunkId{ 0 } {}

        /** Holds the vertex index. */
        unsigned int idx;
        /** Holds the location only vertex index. */
        unsigned int locOnlyIdx;
        /** Holds the vertices chunk id. */
        unsigned int chunkId;
        /** Holds the vertexes triangles. */
        std::vector<unsigned int> triangles;

        template<class Archive>
        void serialize(Archive & ar, const unsigned int)
        {
            ar & idx;
            ar & locOnlyIdx;
            ar & chunkId;
            ar & triangles;
        }
    };

    /** Contains indices for triangles vertices and connectivity. */
    struct MeshConnectTriangle
    {
        MeshConnectTriangle() {}

        explicit MeshConnectTriangle(const std::array<unsigned int, 3>& v, const std::array<unsigned int, 3>& lv) :
            vertex_(v),
            locOnlyVtxIds_(lv),
            neighbors_({ { -1, -1, -1 } })
        {};

        /** Holds the triangles vertices. */
        std::array<unsigned int, 3> vertex_;
        /** Holds the triangles location only vertex ids. */
        std::array<unsigned int, 3> locOnlyVtxIds_;
        /** Holds the triangles neighbors. */
        std::array<int, 3> neighbors_;

        template<class Archive>
        void serialize(Archive & ar, const unsigned int)
        {
            ar & vertex_;
            ar & locOnlyVtxIds_;
            ar & neighbors_;
        }
    };

    /**
     * @brief  Collects connectivity information in meshes.
     *
     * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
     * @date   2016.02.18
     */
    class ConnectivityMesh
    {
    public:
        explicit ConnectivityMesh(const Mesh* mesh);
        ConnectivityMesh(const ConnectivityMesh&);
        ConnectivityMesh& operator=(const ConnectivityMesh&);
        ConnectivityMesh(ConnectivityMesh&&);

        ConnectivityMesh& operator=(ConnectivityMesh&&);
        ~ConnectivityMesh();

        void FindPointsWithinRadius(const glm::vec3 center, float radius, std::vector<unsigned int>& result) const;
        unsigned int FindNearest(const glm::vec3 center) const;
        void FindTrianglesWithinRadius(const glm::vec3 center, float radius, std::vector<unsigned int>& result) const;
        unsigned int FindNearestTriangle(const glm::vec3 center) const;
        unsigned int FindContainingTriangle(const glm::vec3 point) const;
        const std::vector<std::unique_ptr<ConnectivitySubMesh>>& GetSubMeshes() const;

        const std::vector<MeshConnectVertex>& GetVertices() const;
        const std::vector<MeshConnectTriangle>& GetTriangles() const;
        const MeshConnectTriangle& GetTriangle(unsigned int idx) const;

    private:
        std::unique_ptr<impl::ConnectivityMeshImpl> impl_;

    };
}

#endif /* CONNECTIVITYMESH_H */
