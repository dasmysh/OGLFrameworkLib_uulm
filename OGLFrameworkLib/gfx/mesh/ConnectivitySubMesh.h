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

    struct MeshConnectVertex;
    class ConnectivityMesh;

    /**
     * @brief  Collects connectivity information in sub-meshes.
     *
     * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
     * @date   2016.02.18
     */
    class ConnectivitySubMesh
    {
    public:
        ConnectivitySubMesh(const Mesh* mesh, const ConnectivityMesh* cmesh, unsigned int subMeshId, unsigned int triangleRangeStart);
        ConnectivitySubMesh(const ConnectivitySubMesh&);
        ConnectivitySubMesh& operator=(const ConnectivitySubMesh&);
        ConnectivitySubMesh(ConnectivitySubMesh&&);
        ConnectivitySubMesh& operator=(ConnectivitySubMesh&&);
        ~ConnectivitySubMesh();

        // void FindPointsWithinRadius(const glm::vec3 center, float radius, std::vector<>& ) const;
        // unsigned int FindContainingTriangle(const glm::vec3 point) const;
        const SubMesh& GetSubMeshObject() const { return *mesh_->GetSubMesh(subMeshId_); }

        static std::unique_ptr<ConnectivitySubMesh> load(std::ifstream& meshFile, const Mesh* mesh, const ConnectivityMesh* cmesh);
        void save(std::ofstream& ofs) const;

    private:
        explicit ConnectivitySubMesh(const Mesh* mesh, const ConnectivityMesh* cmesh);

        void CreateAABB();

        //unsigned int GetVtxIndex(unsigned int localIndex) const { return cMesh_-> verticesConnect_[localIndex].idx; }
        // unsigned int GetVtxIndex(unsigned int triIdx, unsigned int vtxIdx) const { return  GetVtxIndex(triangleConnect_[triIdx].vertex_[vtxIdx]); }

        /** The mesh to create connectivity from. */
        const Mesh* mesh_;
        /** The connectivity mesh. */
        const ConnectivityMesh* cMesh_;
        /** The id of the sub-mesh to create the connectivity from. */
        unsigned int subMeshId_;
        /** Holds the first triangle index. */
        unsigned int triangleRangeStart_;
        /** Holds the number of triangles. */
        unsigned int numTriangles_;
        /** Holds the bounding box of the sub-meshes. */
        cguMath::AABB3<float> aabb_;

        typedef boost::geometry::model::point<float, 3, boost::geometry::cs::cartesian> point;
        typedef boost::geometry::model::box<point> box;
        typedef boost::geometry::model::polygon<point, false, false> polygon; // CCW, open polygon
        typedef std::pair<box, unsigned> polyIdxBox;

        typedef boost::geometry::index::rtree<polyIdxBox, boost::geometry::index::quadratic<16>> TriangleRTreeType;

        /** Holds the tree for fast finding points in triangles. */
        // TriangleRTreeType triangleFastFindTree_;

    };
}

#endif /* CONNECTIVITYSUBMESH_H */
