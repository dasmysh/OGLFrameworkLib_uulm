/**
 * @file   ConnectivityMesh.cpp
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2016.02.19
 *
 * @brief  Contains the implementation of the connectivity information class for meshes.
 */

#define GLM_SWIZZLE

#include "ConnectivityMesh.h"
#include "ConnectivitySubMesh.h"
#include "core/math/math.h"
#include "SceneMeshNode.h"
#include <boost/filesystem/path.hpp>
#include <fstream>
#include <core/serializationHelper.h>
#include <boost/filesystem/operations.hpp>
#include "SubMesh.h"

#undef min
#undef max


namespace cgu {

    /** Default constructor. */
    ConnectivityMesh::ConnectivityMesh(const Mesh* mesh) :
        mesh_(mesh)
    {
        auto meshFile = mesh->GetFullFilename();
        boost::filesystem::path origPath(meshFile);
        auto connectFilePath = origPath.parent_path().string() + "/" + origPath.stem().string() + "_connectivity.myshbin";
        CreateVertexRTree();

        if (!load(connectFilePath)) CreateNewConnectivity(connectFilePath, mesh);
        CreateTriangleRTree();
    }

    /** Copy constructor. */
    ConnectivityMesh::ConnectivityMesh(const ConnectivityMesh& rhs) :
        mesh_(rhs.mesh_),
        triangleConnect_(rhs.triangleConnect_),
        verticesConnect_(rhs.verticesConnect_),
        aabb_(rhs.aabb_),
        vertexFindTree_(rhs.vertexFindTree_)
    {
        for (unsigned int i = 0; i < mesh_->GetNumSubmeshes(); ++i) {
            subMeshConnectivity_.emplace_back(std::make_unique<ConnectivitySubMesh>(*rhs.GetSubMeshes()[i]));
        }
    }

    /** Copy assignment operator. */
    ConnectivityMesh& ConnectivityMesh::operator=(const ConnectivityMesh& rhs)
    {
        if (this != &rhs) {
            ConnectivityMesh tmp{ rhs };
            std::swap(*this, tmp);
        }
        return *this;
    }

    /** Default move constructor. */
    ConnectivityMesh::ConnectivityMesh(ConnectivityMesh&& rhs) :
        mesh_(std::move(rhs.mesh_)),
        triangleConnect_(std::move(rhs.triangleConnect_)),
        verticesConnect_(std::move(rhs.verticesConnect_)),
        aabb_(std::move(rhs.aabb_)),
        subMeshConnectivity_(std::move(rhs.subMeshConnectivity_)),
        vertexFindTree_(std::move(rhs.vertexFindTree_))
    {
    }

    /** Default move assignment operator. */
    ConnectivityMesh& ConnectivityMesh::operator=(ConnectivityMesh&& rhs)
    {
        if (this != &rhs) {
            this->~ConnectivityMesh();
            mesh_ = std::move(rhs.mesh_);
            triangleConnect_ = std::move(rhs.triangleConnect_);
            verticesConnect_ = std::move(rhs.verticesConnect_);
            aabb_ = std::move(rhs.aabb_);
            subMeshConnectivity_ = std::move(rhs.subMeshConnectivity_);
            vertexFindTree_ = std::move(rhs.vertexFindTree_);
        }
        return *this;
    }

    /** Default destructor. */
    ConnectivityMesh::~ConnectivityMesh() = default;

    void ConnectivityMesh::CreateNewConnectivity(const std::string& connectFilePath, const Mesh* mesh)
    {
        std::vector<unsigned int> reducedVertexMap(mesh->GetVertices().size(), static_cast<unsigned int>(mesh->GetVertices().size()));

        cguMath::AABB3<float> meshAABB;
        mesh->GetRootNode()->GetBoundingBox(meshAABB, glm::mat4(1.0f));
        auto querySize = (meshAABB.minmax[1] - meshAABB.minmax[0]) / 20000.0f;

        for (auto i = 0; i < mesh->GetVertices().size(); ++i) {
            if (reducedVertexMap[i] != mesh->GetVertices().size()) continue;

            typedef boost::geometry::model::box<point> box;
            const auto& v = mesh->GetVertices()[i];
            box queryBox(point(v.x - querySize.x, v.y - querySize.y, v.z - querySize.z), point(v.x + querySize.x, v.y + querySize.y, v.z + querySize.z));
            namespace bgi = boost::geometry::index;
            for (const auto& qr : vertexFindTree_ | bgi::adaptors::queried(bgi::within(queryBox))) {
                if (mesh->GetVertices()[i] == mesh->GetVertices()[qr.second]) reducedVertexMap[qr.second] = i;
            }
        }

        verticesConnect_.resize(mesh_->GetVertices().size());

        for (unsigned int smI = 0; smI < mesh_->GetNumSubmeshes(); ++smI) {
            auto firstIdx = FillSubmeshConnectivity(smI, reducedVertexMap);

            subMeshConnectivity_.emplace_back(std::make_unique<ConnectivitySubMesh>(mesh_, this, smI, firstIdx));
        }

        CalculateChunkIds();
        mesh_->GetRootNode()->GetBoundingBox(aabb_, mesh_->GetRootTransform());
        save(connectFilePath);
    }

    void ConnectivityMesh::FindPointsWithinRadius(const glm::vec3 center, float radius, std::vector<unsigned>& result) const
    {
        typedef boost::geometry::model::box<point> box;
        auto minPt = center - glm::vec3(radius);
        auto maxPt = center + glm::vec3(radius);
        box queryBox(point(minPt.x, minPt.y, minPt.z), point(maxPt.x, maxPt.y, maxPt.z));
        namespace bgi = boost::geometry::index;
        // vertexFindTree_.query(bgi::within(queryBox), std::back_inserter(result));
        for (const auto& qr : vertexFindTree_ | bgi::adaptors::queried(bgi::within(queryBox))) result.push_back(qr.second);
            //if (i != qr.second && mesh->GetVertices()[i] == mesh->GetVertices()[qr.second]) reducedVertexMap[qr.second] = i;
        //}
    }

    unsigned int ConnectivityMesh::FillSubmeshConnectivity(unsigned int smI, const std::vector<unsigned int>& reducedVertexMap)
    {
        const auto& subMesh = mesh_->GetSubMesh(smI);
        auto firstIndex = static_cast<unsigned int>(triangleConnect_.size());
        auto lastIndex = subMesh->GetIndexOffset() + subMesh->GetNumberOfIndices();

        for (auto i = subMesh->GetIndexOffset(); i < lastIndex; i += 3) {
            std::array<unsigned int, 3> indices = { mesh_->GetIndices()[i], mesh_->GetIndices()[i + 1], mesh_->GetIndices()[i + 2] };
            std::array<unsigned int, 3> localIndices = { reducedVertexMap[indices[0]], reducedVertexMap[indices[1]], reducedVertexMap[indices[2]] };
            triangleConnect_.emplace_back(indices, localIndices);
        }

        unsigned int cid = 0;
        for (unsigned int idx = 0; idx < verticesConnect_.size(); ++idx) {
            verticesConnect_[cid].idx = idx;
            verticesConnect_[cid++].locOnlyIdx = reducedVertexMap[idx];
        }

        // set vertex connectivity
        for (auto i = 0; i < triangleConnect_.size(); ++i) {
            auto& tri = triangleConnect_[i];

            for (auto j = 0; j < 3; ++j) {
                auto& vl = verticesConnect_[tri.locOnlyVtxIds_[j]].triangles;
                if (std::find(vl.begin(), vl.end(), i) == vl.end()) vl.push_back(i);

                auto& vg = verticesConnect_[tri.vertex_[j]].triangles;
                if (std::find(vg.begin(), vg.end(), i) == vg.end()) vg.push_back(i);
            }
        }

        // set triangle neighbors
        for (auto i = 0; i < triangleConnect_.size(); ++i) {
            auto& tri = triangleConnect_[i];
            for (unsigned int ni = 0; ni < 3; ++ni) {
                auto vi0 = tri.locOnlyVtxIds_[(ni + 1) % 3];
                auto vi1 = tri.locOnlyVtxIds_[(ni + 2) % 3];

                if (vi0 == vi1) {
                    vi0 = tri.vertex_[(ni + 1) % 3];
                    vi1 = tri.vertex_[(ni + 2) % 3];
                }
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
        return firstIndex;
    }

    /**
     *  Find index of triangle that contains the given point.
     *  @param point the point to find the triangle for.
     */
    unsigned int ConnectivityMesh::FindContainingTriangle(const glm::vec3 pt)
    {
        if (!cguMath::pointInAABB3Test(aabb_, pt)) return -1;

        // std::vector<polyIdxBox> hits;
        namespace bg = boost::geometry;
        //triangleFastFindTree_.query(bg::index::contains(point(pt.x, pt.y, pt.z)), std::back_inserter(hits));
        auto& vertices = mesh_->GetVertices();
        for (const auto& polyBox : triangleFastFindTree_ | bg::index::adaptors::queried(bg::index::contains(point(pt.x, pt.y, pt.z)))) {
            auto triId = polyBox.second;
            cguMath::Tri3<float> tri{ { vertices[triangleConnect_[triId].vertex_[0]].xyz(), vertices[triangleConnect_[triId].vertex_[1]].xyz(),
                vertices[triangleConnect_[triId].vertex_[2]].xyz() } };
            if (cguMath::pointInTriangleTest<float>(tri, pt, nullptr)) return triId;
        }
        throw std::out_of_range("Containing triangle not found!");


        /*for (auto& submesh : subMeshConnectivity_) {
            auto result = submesh->FindContainingTriangle(point);
            if (result != -1) return result;
        }
        throw std::out_of_range("Containing triangle not found!");*/
    }

    bool ConnectivityMesh::load(const std::string& meshFile)
    {
        if (boost::filesystem::exists(meshFile)) {
            std::ifstream inBinFile(meshFile, std::ios::binary);
            if (inBinFile.is_open()) {
                bool correctHeader;
                unsigned int actualVersion;
                std::tie(correctHeader, actualVersion) = VersionableSerializerType::checkHeader(inBinFile);
                if (correctHeader) {
                    serializeHelper::read(inBinFile, aabb_.minmax[0]);
                    serializeHelper::read(inBinFile, aabb_.minmax[1]);

                    unsigned int triSize, vtxSize;
                    serializeHelper::read(inBinFile, triSize);
                    triangleConnect_.resize(triSize);
                    for (auto& tri : triangleConnect_) {
                        serializeHelper::read(inBinFile, tri.vertex_[0]);
                        serializeHelper::read(inBinFile, tri.vertex_[1]);
                        serializeHelper::read(inBinFile, tri.vertex_[2]);
                        serializeHelper::read(inBinFile, tri.locOnlyVtxIds_[0]);
                        serializeHelper::read(inBinFile, tri.locOnlyVtxIds_[1]);
                        serializeHelper::read(inBinFile, tri.locOnlyVtxIds_[2]);
                        serializeHelper::read(inBinFile, tri.neighbors_[0]);
                        serializeHelper::read(inBinFile, tri.neighbors_[1]);
                        serializeHelper::read(inBinFile, tri.neighbors_[2]);
                    }

                    serializeHelper::read(inBinFile, vtxSize);
                    verticesConnect_.resize(vtxSize);
                    for (auto& vtx : verticesConnect_) {
                        serializeHelper::read(inBinFile, vtx.idx);
                        serializeHelper::read(inBinFile, vtx.locOnlyIdx);
                        serializeHelper::read(inBinFile, vtx.chunkId);
                        serializeHelper::readV(inBinFile, vtx.triangles);
                    }

                    unsigned int numSubmeshes;
                    serializeHelper::read(inBinFile, numSubmeshes);
                    subMeshConnectivity_.resize(numSubmeshes);
                    for (auto& submesh : subMeshConnectivity_) {
                        bool loadedCorrectly;
                        std::tie(submesh, loadedCorrectly) = std::move(ConnectivitySubMesh::load(inBinFile, mesh_, this));
                        if (!loadedCorrectly) return false;
                    }
                    return true;
                }
            }
        }
        return false;
    }

    void ConnectivityMesh::save(const std::string& meshFile) const
    {
        std::ofstream ofs(meshFile, std::ios::out | std::ios::binary);
        VersionableSerializerType::writeHeader(ofs);
        serializeHelper::write(ofs, aabb_.minmax[0]);
        serializeHelper::write(ofs, aabb_.minmax[1]);

        serializeHelper::write(ofs, static_cast<unsigned>(triangleConnect_.size()));
        for (const auto& tri : triangleConnect_) {
            serializeHelper::write(ofs, tri.vertex_[0]);
            serializeHelper::write(ofs, tri.vertex_[1]);
            serializeHelper::write(ofs, tri.vertex_[2]);
            serializeHelper::write(ofs, tri.locOnlyVtxIds_[0]);
            serializeHelper::write(ofs, tri.locOnlyVtxIds_[1]);
            serializeHelper::write(ofs, tri.locOnlyVtxIds_[2]);
            serializeHelper::write(ofs, tri.neighbors_[0]);
            serializeHelper::write(ofs, tri.neighbors_[1]);
            serializeHelper::write(ofs, tri.neighbors_[2]);
        }

        serializeHelper::write(ofs, static_cast<unsigned>(verticesConnect_.size()));
        for (const auto& vtx : verticesConnect_) {
            serializeHelper::write(ofs, vtx.idx);
            serializeHelper::write(ofs, vtx.locOnlyIdx);
            serializeHelper::write(ofs, vtx.chunkId);
            serializeHelper::writeV(ofs, vtx.triangles);
        }

        serializeHelper::write(ofs, static_cast<unsigned int>(subMeshConnectivity_.size()));
        for (auto& submesh : subMeshConnectivity_) submesh->save(ofs);
    }

    void ConnectivityMesh::CreateVertexRTree()
    {
        const auto& vertices = mesh_->GetVertices();
        for (unsigned int i = 0; i < vertices.size(); ++i) {
            const auto& vtx = vertices[i];
            vertexFindTree_.insert(std::make_pair(point(vtx.x, vtx.y, vtx.z), i));
        }
    }

    /**
    *  Creates the sub meshes r-tree for faster point in triangle tests.
    */
    void ConnectivityMesh::CreateTriangleRTree()
    {
        auto& vertices = mesh_->GetVertices();
        for (auto i = 0; i < triangleConnect_.size(); ++i) {
            polygon poly;
            for (unsigned int vi = 0; vi < 3; ++vi) {
                auto vpt = vertices[triangleConnect_[i].vertex_[vi]].xyz();
                poly.outer().push_back(point(vpt.x, vpt.y, vpt.z));
            }
            auto b = boost::geometry::return_envelope<box>(poly);
            triangleFastFindTree_.insert(std::make_pair(b, i));
        }
    }

    void ConnectivityMesh::CalculateChunkIds()
    {
        // invalidate all chunk ids.
        auto invalidId = static_cast<unsigned int>(mesh_->GetVertices().size());
        for (auto& vtx : verticesConnect_) vtx.chunkId = invalidId;

        unsigned int currentId = 0;
        for (auto& vtx : verticesConnect_) {
            if (vtx.chunkId != invalidId) continue;

            if (vtx.idx != vtx.locOnlyIdx) {
                MarkVertexForChunk(vtx, verticesConnect_[vtx.locOnlyIdx].chunkId);
            } else {
                MarkVertexForChunk(vtx, currentId);

                ++currentId;
            }
        }
    }

    void ConnectivityMesh::MarkVertexForChunk(MeshConnectVertex& vtx, unsigned int chunkId)
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
