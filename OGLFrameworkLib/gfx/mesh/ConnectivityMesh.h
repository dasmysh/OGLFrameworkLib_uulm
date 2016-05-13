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
#include <core/math/math.h>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/geometries/polygon.hpp>

namespace cgu {

    class Mesh;
    class ConnectivitySubMesh;


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
        unsigned int FindContainingTriangle(const glm::vec3 point);
        const std::vector<std::unique_ptr<ConnectivitySubMesh>>& GetSubMeshes() const { return subMeshConnectivity_; }

        const std::vector<MeshConnectVertex>& GetVertices() const { return verticesConnect_; }
        const std::vector<MeshConnectTriangle>& GetTriangles() const { return triangleConnect_; }
        const MeshConnectTriangle& GetTriangle(unsigned int idx) const { return triangleConnect_[idx]; }

    private:
        void load(const std::string& meshFile);
        void save(const std::string& meshFile) const;

        void CreateVertexRTree();
        void CreateTriangleRTree();
        unsigned int FillSubmeshConnectivity(unsigned int smI, const std::vector<unsigned int>& reducedVertexMap);
        void CalculateChunkIds();
        void MarkVertexForChunk(MeshConnectVertex& vtx, unsigned int chunkId);

        /** The mesh to create connectivity from. */
        const Mesh* mesh_;
        /** Holds a list of triangles with connectivity information. */
        std::vector<MeshConnectTriangle> triangleConnect_;
        /** Holds a list of vertex connectivity information. */
        std::vector<MeshConnectVertex> verticesConnect_;
        /** Contains a bounding box containing all sub-meshes. */
        cguMath::AABB3<float> aabb_;
        /** Connectivity information for the sub-meshes. */
        std::vector<std::unique_ptr<ConnectivitySubMesh>> subMeshConnectivity_;

        typedef boost::geometry::model::point<float, 3, boost::geometry::cs::cartesian> point;
        typedef boost::geometry::model::box<point> box;
        typedef boost::geometry::model::polygon<point, false, false> polygon; // CCW, open polygon
        typedef std::pair<box, unsigned> polyIdxBox;
        typedef std::pair<point, unsigned> vtxIdxPoint;
        typedef boost::geometry::index::rtree<vtxIdxPoint, boost::geometry::index::quadratic<16>> VertexRTreeType;
        typedef boost::geometry::index::rtree<polyIdxBox, boost::geometry::index::quadratic<16>> TriangleRTreeType;

        /** Holds the tree for fast finding vertices and nearest neighbors. */
        VertexRTreeType vertexFindTree_;
        /** Holds the tree for fast finding points in triangles. */
        TriangleRTreeType triangleFastFindTree_;
    };
}

#endif /* CONNECTIVITYMESH_H */
