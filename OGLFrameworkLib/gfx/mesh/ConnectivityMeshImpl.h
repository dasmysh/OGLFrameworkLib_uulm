/**
 * @file   ConnectivityMeshImpl.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.06.17
 *
 * @brief  Defines the connectivity mesh implementation for PIMPL scheme.
 */

#ifndef CONNECTIVITYMESHIMPL_H
#define CONNECTIVITYMESHIMPL_H

#define BOOST_GEOMETRY_INDEX_DETAIL_EXPERIMENTAL

#include "main.h"
#include <core/math/math.h>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <core/serializationHelper.h>
#include <boost/serialization/version.hpp>
// ReSharper disable CppUnusedIncludeDirective
#include <boost/serialization/vector.hpp>
#include <boost/serialization/unique_ptr.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
// ReSharper restore CppUnusedIncludeDirective

namespace cgu {

    class Mesh;
    class ConnectivitySubMesh;
    struct MeshConnectVertex;
    struct MeshConnectTriangle;

    namespace impl {

        class ConnectivityMeshImpl
        {
        public:
            explicit ConnectivityMeshImpl(const Mesh* mesh);
            ConnectivityMeshImpl(const ConnectivityMeshImpl&);
            ConnectivityMeshImpl& operator=(const ConnectivityMeshImpl&);
            ConnectivityMeshImpl(ConnectivityMeshImpl&&);

            ConnectivityMeshImpl& operator=(ConnectivityMeshImpl&&);
            ~ConnectivityMeshImpl();

            void FindPointsWithinRadius(const glm::vec3 center, float radius, std::vector<unsigned int>& result) const;
            unsigned int FindNearest(const glm::vec3 center) const;
            void FindTrianglesWithinRadius(const glm::vec3 center, float radius, std::vector<unsigned int>& result) const;
            unsigned int FindNearestTriangle(const glm::vec3 center) const;
            unsigned int FindContainingTriangle(const glm::vec3 point);
            const std::vector<std::unique_ptr<ConnectivitySubMesh>>& GetSubMeshes() const { return subMeshConnectivity_; }

            const std::vector<MeshConnectVertex>& GetVertices() const { return verticesConnect_; }
            const std::vector<MeshConnectTriangle>& GetTriangles() const { return triangleConnect_; }
            const MeshConnectTriangle& GetTriangle(unsigned int idx) const { return triangleConnect_[idx]; }

        private:
            friend class boost::serialization::access;
            using VersionableSerializerType = serializeHelper::VersionableSerializer<'C', 'N', 'T', 'M', 1001>;

            template<class Archive>
            void serialize(Archive & ar, const unsigned int version)
            {
                ar & vertexFindTree_;
                ar & aabb_.minmax;
                ar & triangleConnect_;
                ar & verticesConnect_;
                ar & subMeshConnectivity_;
                ar & triangleFastFindTree_;

                if (version > 1) {} // do things here...
            }

            bool load(const std::string& meshFile);
            bool loadV1(std::ifstream& inBinFile);
            void save(const std::string& meshFile);

            void CreateNewConnectivity(const std::string& connectFilePath, const Mesh* mesh);
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

        public:
            typedef boost::geometry::model::point<float, 3, boost::geometry::cs::cartesian> point;
            typedef boost::geometry::model::box<point> box;
            typedef boost::geometry::model::polygon<point, false, false> polygon; // CCW, open polygon
            typedef std::pair<box, size_t> polyIdxBox;
            typedef std::pair<point, size_t> vtxIdxPoint;

        private:
            typedef boost::geometry::index::rtree<vtxIdxPoint, boost::geometry::index::quadratic<16, 2>> VertexRTreeType;
            typedef boost::geometry::index::rtree<polyIdxBox, boost::geometry::index::quadratic<16, 2>> TriangleRTreeType;

            /** Holds the tree for fast finding vertices and nearest neighbors. */
            VertexRTreeType vertexFindTree_;
            /** Holds the tree for fast finding points in triangles. */
            TriangleRTreeType triangleFastFindTree_;
        };

        namespace serialization {

            struct iarchive : public boost::archive::binary_iarchive {
                iarchive(std::istream& is, const Mesh* mesh, const ConnectivityMeshImpl* cmesh) :
                    boost::archive::binary_iarchive(is), mesh_(mesh), cmesh_(cmesh) {}

                const Mesh* mesh_;
                const ConnectivityMeshImpl* cmesh_;
            };

        }
    }
}

namespace boost {
    namespace serialization {

        template<class Archive>
        void serialize(Archive & ar, std::pair<cgu::impl::ConnectivityMeshImpl::point, size_t>& g, const unsigned int)
        {
            ar & g.first;
            ar & g.second;
        }

        template<class Archive>
        void serialize(Archive & ar, std::pair<cgu::impl::ConnectivityMeshImpl::box, size_t>& g, const unsigned int)
        {
            ar & g.first;
            ar & g.second;
        }
    }
}

BOOST_CLASS_VERSION(cgu::impl::ConnectivityMeshImpl, 1)

#endif // CONNECTIVITYMESHIMPL_H
