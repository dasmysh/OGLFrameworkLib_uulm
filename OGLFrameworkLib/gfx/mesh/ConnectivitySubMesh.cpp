/**
 * @file   Mesh.cpp
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2014.01.13
 *
 * @brief  Contains the implementation of a class defining connectivity information in sub-meshes.
 */

#define GLM_SWIZZLE

#include "ConnectivitySubMesh.h"
#include "core/math/math.h"
#include <boost/geometry/index/rtree.hpp>
#include "SubMesh.h"

#undef min
#undef max


namespace cgu {

    /** Default constructor. */
    ConnectivitySubMesh::ConnectivitySubMesh(const Mesh* mesh, const std::vector<unsigned int>& localIdxMap, unsigned int subMeshId) :
        mesh_(mesh),
        subMeshId_(subMeshId)
    {
        auto subMesh = mesh_->GetSubMesh(subMeshId_);

        auto lastIndex = subMesh->GetIndexOffset() + subMesh->GetNumberOfIndices();

        std::set<unsigned int> indexSet;
        for (auto i = subMesh->GetIndexOffset(); i < lastIndex; i += 3) {
            std::array<unsigned int, 3> indices = { mesh_->GetIndices()[i], mesh_->GetIndices()[i + 1], mesh_->GetIndices()[i + 2] };
            std::array<unsigned int, 3> localIndices = { localIdxMap[indices[0]], localIdxMap[indices[1]], localIdxMap[indices[2]] };
            indexSet.insert(indices[0]);
            indexSet.insert(indices[1]);
            indexSet.insert(indices[2]);
            triangleConnect_.emplace_back(indices, localIndices);
        }

        // std::unordered_map<unsigned int, unsigned int> localVertexMap;

        verticesConnect_.resize(indexSet.size());
        unsigned int cid = 0;
        for (auto& idx : indexSet) {
            // localVertexMap.insert(std::make_pair(idx, cid++));
            verticesConnect_[idx].idx = idx;
            verticesConnect_[idx].locOnlyIdx = localIdxMap[idx];
        }

        // set vertex connectivity
        for (auto i = 0; i < triangleConnect_.size(); ++i) {
            auto& tri = triangleConnect_[i];
            /*tri.vertex[0] = localVertexMap[tri.vertex[0]];
            tri.vertex[1] = localVertexMap[tri.vertex[1]];
            tri.vertex[2] = localVertexMap[tri.vertex[2]];*/
            std::array<bool, 3> insertVtx{ { false, false, false } };
            
            for (auto j = 0; j < 3; ++j) {
                auto& vl = verticesConnect_[tri.locOnlyVtxIds_[0]].triangles;
                if (std::find(vl.begin(), vl.end(), i) == vl.end()) vl.push_back(i);
                
                auto& vg = verticesConnect_[tri.vertex_[0]].triangles;
                if (std::find(vg.begin(), vg.end(), i) == vg.end()) vg.push_back(i);
            }
            /*verticesConnect_[tri.locOnlyVtxIds_[0]].triangles.push_back(i);
            verticesConnect_[tri.locOnlyVtxIds_[1]].triangles.push_back(i);
            verticesConnect_[tri.locOnlyVtxIds_[2]].triangles.push_back(i);*/
        }

        // set triangle neighbors
        for (auto i = 0; i < triangleConnect_.size(); ++i) {
            auto& tri = triangleConnect_[i];
            for (unsigned int ni = 0; ni < 3; ++ni) {
                auto vi0 = tri.locOnlyVtxIds_[(ni + 1) % 3];
                auto vi1 = tri.locOnlyVtxIds_[(ni + 2) % 3];
                std::vector<unsigned int> isect;
                std::set_intersection(verticesConnect_[vi0].triangles.begin(), verticesConnect_[vi0].triangles.end(),
                    verticesConnect_[vi1].triangles.begin(), verticesConnect_[vi1].triangles.end(), std::back_inserter(isect));
                // if the mesh is planar and has borders only ONE triangle may be found!!!
                // this triangle is the triangle itself not its neighbor
                assert(isect.size() <= 2); // both this triangle and the neighbor should be found
                if (isect.size() == 2) {
                    tri.neighbors_[ni] = isect[0] == i ? isect[1] : isect[0];
                } else {
                    tri.neighbors_[ni] = -1;
                }
            }
        }

        CalculateChunkIds();
        CreateAABB();
        CreateRTree();
    }

    /** Copy constructor. */
    ConnectivitySubMesh::ConnectivitySubMesh(const ConnectivitySubMesh& rhs) :
        mesh_(rhs.mesh_),
        subMeshId_(rhs.subMeshId_),
        triangleConnect_(rhs.triangleConnect_),
        verticesConnect_(rhs.verticesConnect_),
        aabb_(rhs.aabb_),
        fastFindTree_(rhs.fastFindTree_)
    {
    }

    /** Copy assignment operator. */
    ConnectivitySubMesh& ConnectivitySubMesh::operator=(const ConnectivitySubMesh& rhs)
    {
        if (this != &rhs) {
            ConnectivitySubMesh tmp{ rhs };
            std::swap(*this, tmp);
        }
        return *this;
    }

    /** Default move constructor. */
    ConnectivitySubMesh::ConnectivitySubMesh(ConnectivitySubMesh&& rhs) :
        mesh_(std::move(rhs.mesh_)),
        subMeshId_(std::move(rhs.subMeshId_)),
        triangleConnect_(std::move(rhs.triangleConnect_)),
        verticesConnect_(std::move(rhs.verticesConnect_)),
        aabb_(std::move(rhs.aabb_)),
        fastFindTree_(std::move(rhs.fastFindTree_))
    {
    }

    /** Default move assignment operator. */
    ConnectivitySubMesh& ConnectivitySubMesh::operator=(ConnectivitySubMesh&& rhs)
    {
        if (this != &rhs) {
            this->~ConnectivitySubMesh();
            mesh_ = std::move(rhs.mesh_);
            subMeshId_ = std::move(rhs.subMeshId_);
            triangleConnect_ = std::move(rhs.triangleConnect_);
            verticesConnect_ = std::move(rhs.verticesConnect_);
            aabb_ = std::move(rhs.aabb_);
            fastFindTree_ = std::move(rhs.fastFindTree_);
        }
        return *this;
    }

    /** Default destructor. */
    ConnectivitySubMesh::~ConnectivitySubMesh() = default;

    /**
     *  Find index of triangle that contains the given point.
     *  @param pt the point to find the triangle for.
     */
    unsigned int ConnectivitySubMesh::FindContainingTriangle(const glm::vec3 pt) const
    {
        if (!cguMath::pointInAABB3Test(aabb_, pt)) return -1;

        std::vector<polyIdxBox> hits;
        namespace bg = boost::geometry;
        fastFindTree_.query(bg::index::contains(point(pt.x, pt.y, pt.z)), std::back_inserter(hits));
        auto& vertices = mesh_->GetVertices();
        for (const auto& polyBox : hits) {
            auto triId = polyBox.second;
            cguMath::Tri3<float> tri{ { vertices[GetVtxIndex(triId, 0)].xyz(), vertices[GetVtxIndex(triId, 1)].xyz(),
                vertices[GetVtxIndex(triId, 2)].xyz() } };
            if (cguMath::pointInTriangleTest<float>(tri, pt, nullptr)) return triId;
        }
        return -1;
    }

    /**
     *  Creates the sub meshes bounding box.
     */
    void ConnectivitySubMesh::CreateAABB()
    {
        auto& vertices = mesh_->GetVertices();
        if (triangleConnect_.empty()) return;
        aabb_.minmax[0] = aabb_.minmax[1] = vertices[GetVtxIndex(0, 0)].xyz();
        for (const auto& tri : triangleConnect_) {
            for (auto vi : tri.locOnlyVtxIds_) {
                aabb_.minmax[0] = glm::min(aabb_.minmax[0], vertices[vi].xyz());
                aabb_.minmax[1] = glm::max(aabb_.minmax[1], vertices[vi].xyz());
            }
        }
    }

    /**
     *  Creates the sub meshes r-tree for faster point in triangle tests.
     */
    void ConnectivitySubMesh::CreateRTree()
    {
        auto& vertices = mesh_->GetVertices();
        for (auto i = 0; i < triangleConnect_.size(); ++i) {
            polygon poly;
            for (unsigned int vi = 0; vi < 3; ++vi) {
                auto vpt = vertices[GetVtxIndex(i, vi)].xyz();
                poly.outer().push_back(point(vpt.x, vpt.y, vpt.z));
            }
            auto b = boost::geometry::return_envelope<box>(poly);
            fastFindTree_.insert(std::make_pair(b, i));
        }
    }

    void ConnectivitySubMesh::CalculateChunkIds()
    {
        // invalidate all chunk ids.
        auto invalidId = static_cast<unsigned int>(verticesConnect_.size());
        for (auto& vtx : verticesConnect_) vtx.chunkId = invalidId;

        unsigned int currentId = 0;
        for (auto& vtx : verticesConnect_) {
            if (vtx.chunkId != invalidId) continue;

            if (vtx.idx == vtx.locOnlyIdx) {
                MarkVertexForChunk(vtx, verticesConnect_[vtx.locOnlyIdx].chunkId);
            } else {
                MarkVertexForChunk(vtx, currentId);

                ++currentId;
            }
        }

        //for (auto& vtx : verticesConnect_) {
            // propagate local chunk ids to global vertices
        //}
    }

    void ConnectivitySubMesh::MarkVertexForChunk(MeshConnectVertex& vtx, unsigned int chunkId)
    {
        if (vtx.chunkId == chunkId) return;

        std::queue<unsigned int> workingChunkQueue;
        workingChunkQueue.push(vtx.idx);

        while (!workingChunkQueue.empty()) {
            auto vId = workingChunkQueue.front();
            workingChunkQueue.pop();

            if (verticesConnect_[vId].chunkId == chunkId) continue;

            verticesConnect_[vId].chunkId = chunkId;
            for (auto tId : verticesConnect_[vId].triangles) {
                for (auto vIdNext : triangleConnect_[tId].locOnlyVtxIds_) {
                    workingChunkQueue.push(vIdNext);
                }
            }

        }
    }
}
