/**
 * @file   ConnectivitySubMesh.h
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2016.02.18
 *
 * @brief  Contains the definition of a class defining connectivity information in sub-meshes.
 */

#ifndef CONNECTIVITYSUBMESH_H
#define CONNECTIVITYSUBMESH_H

#include "main.h"
#include "Mesh.h"
#include <core/math/math.h>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/index/rtree.hpp>

namespace cgu {

    /** Contains vertex connectivity information. */
    struct MeshConnectVertex
    {
        MeshConnectVertex() : idx{ 0 }, chunkId{ 0 } {}

        /** Holds the vertex index. */
        unsigned int idx;
        /** Holds the vertices chunk id. */
        unsigned int chunkId;
        /** Holds the vertexes triangles. */
        std::vector<unsigned int> triangles;
    };

    /** Contains indices for triangles vertices and connectivity. */
    struct MeshConnectTriangle
    {
        MeshConnectTriangle() {}

        explicit MeshConnectTriangle(const std::array<unsigned int, 3>& v) :
            vertex({ { v[0], v[1], v[2] } }),
            neighbors({ { -1, -1, -1 } })
        {};

        bool operator<(const MeshConnectTriangle& rhs) const {
            if (vertex[0] < rhs.vertex[0]) return true;
            if (vertex[0] == rhs.vertex[0]) {
                if (vertex[1] < rhs.vertex[1]) return true;
                if (vertex[1] == rhs.vertex[1]) {
                    if (vertex[2] < rhs.vertex[2]) return true;
                }
            }
            return false;
        };

        bool operator==(const MeshConnectTriangle& rhs) const {
            return rhs.vertex[0] == vertex[0]
                && rhs.vertex[1] == vertex[1]
                && rhs.vertex[2] == vertex[2];
        };

        /** Holds the triangles vertices. */
        std::array<unsigned int, 3> vertex;
        /** Holds the triangles neighbors. */
        std::array<int, 3> neighbors;
    };

    /**
     * @brief  Collects connectivity information in sub-meshes.
     *
     * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
     * @date   2016.02.18
     */
    class ConnectivitySubMesh
    {
    public:
        ConnectivitySubMesh(const Mesh* mesh, unsigned int subMeshId);
        ConnectivitySubMesh(const ConnectivitySubMesh&);
        ConnectivitySubMesh& operator=(const ConnectivitySubMesh&);
        ConnectivitySubMesh(ConnectivitySubMesh&&);
        ConnectivitySubMesh& operator=(ConnectivitySubMesh&&);
        ~ConnectivitySubMesh();

        unsigned int FindContainingTriangle(const glm::vec3 point);
        const std::vector<MeshConnectVertex>& GetVertices() const { return verticesConnect_; }
        const MeshConnectTriangle& GetTriangle(unsigned int idx) const { return triangleConnect_[idx]; }
        const SubMesh& GetSubMeshObject() const { return *mesh_->GetSubMesh(subMeshId_); }

    private:
        void CreateAABB();
        void CreateRTree();
        void CalculateChunkIds();
        void MarkVertexForChunk(MeshConnectVertex& vtx, unsigned int chunkId);

        unsigned int GetVtxIndex(unsigned int localIndex) const { return verticesConnect_[localIndex].idx; }
        unsigned int GetVtxIndex(unsigned int triIdx, unsigned int vtxIdx) const { return GetVtxIndex(triangleConnect_[triIdx].vertex[vtxIdx]); }

        /** The mesh to create connectivity from. */
        const Mesh* mesh_;
        /** The id of the sub-mesh to create the connectivity from. */
        unsigned int subMeshId_;
        /** Holds a list of triangles with connectivity information. */
        std::vector<MeshConnectTriangle> triangleConnect_;
        /** Holds a list of vertex connectivity information. */
        std::vector<MeshConnectVertex> verticesConnect_;
        /** Holds the bounding box of the sub-meshes. */
        cguMath::AABB3<float> aabb_;

        typedef boost::geometry::model::point<float, 3, boost::geometry::cs::cartesian> point;
        typedef boost::geometry::model::box<point> box;
        typedef boost::geometry::model::polygon<point, false, false> polygon; // CCW, open polygon
        typedef std::pair<box, unsigned> polyIdxBox;

        typedef boost::geometry::index::rtree<polyIdxBox, boost::geometry::index::quadratic<16>> RTreeType;

        /** Holds the tree for fast finding points in triangles. */
        RTreeType fastFindTree_;

    };
}

#endif /* CONNECTIVITYSUBMESH_H */
