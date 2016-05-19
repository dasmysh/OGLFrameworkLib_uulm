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
#include <core/serializationHelper.h>
#include "ConnectivityMesh.h"

#undef min
#undef max


namespace cgu {


    ConnectivitySubMesh::ConnectivitySubMesh(const Mesh* mesh, const ConnectivityMesh* cmesh) :
        mesh_(mesh),
        cMesh_(cmesh),
        subMeshId_(0),
        triangleRangeStart_(0),
        numTriangles_(0)
    {
    }

    /** Default constructor. */
    ConnectivitySubMesh::ConnectivitySubMesh(const Mesh* mesh, const ConnectivityMesh* cmesh, unsigned int subMeshId, unsigned int triangleRangeStart) :
        mesh_(mesh),
        cMesh_(cmesh),
        subMeshId_(subMeshId),
        triangleRangeStart_(triangleRangeStart),
        numTriangles_(mesh->GetSubMesh(subMeshId_)->GetNumberOfTriangles())
    {
        CreateAABB();
        // CreateTriangleRTree();
    }

    /** Copy constructor. */
    ConnectivitySubMesh::ConnectivitySubMesh(const ConnectivitySubMesh& rhs) :
        mesh_(rhs.mesh_),
        cMesh_(rhs.cMesh_),
        subMeshId_(rhs.subMeshId_),
        triangleRangeStart_(rhs.triangleRangeStart_),
        numTriangles_(rhs.numTriangles_),
        aabb_(rhs.aabb_)
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
        cMesh_(std::move(rhs.cMesh_)),
        subMeshId_(std::move(rhs.subMeshId_)),
        triangleRangeStart_(std::move(rhs.triangleRangeStart_)),
        numTriangles_(std::move(rhs.numTriangles_)),
        aabb_(std::move(rhs.aabb_))
    {
    }

    /** Default move assignment operator. */
    ConnectivitySubMesh& ConnectivitySubMesh::operator=(ConnectivitySubMesh&& rhs)
    {
        if (this != &rhs) {
            this->~ConnectivitySubMesh();
            mesh_ = std::move(rhs.mesh_);
            cMesh_ = std::move(rhs.cMesh_),
            subMeshId_ = std::move(rhs.subMeshId_);
            triangleRangeStart_ = std::move(rhs.triangleRangeStart_);
            numTriangles_ = std::move(rhs.numTriangles_);
            aabb_ = std::move(rhs.aabb_);
        }
        return *this;
    }

    /** Default destructor. */
    ConnectivitySubMesh::~ConnectivitySubMesh() = default;

    /**
     *  Find index of triangle that contains the given point.
     *  @param pt the point to find the triangle for.
     */
    /*unsigned int ConnectivitySubMesh::FindContainingTriangle(const glm::vec3 pt) const
    {
        if (!cguMath::pointInAABB3Test(aabb_, pt)) return -1;

        std::vector<polyIdxBox> hits;
        namespace bg = boost::geometry;
        triangleFastFindTree_.query(bg::index::contains(point(pt.x, pt.y, pt.z)), std::back_inserter(hits));
        auto& vertices = mesh_->GetVertices();
        for (const auto& polyBox : hits) {
            auto triId = polyBox.second;
            cguMath::Tri3<float> tri{ { vertices[GetVtxIndex(triId, 0)].xyz(), vertices[GetVtxIndex(triId, 1)].xyz(),
                vertices[GetVtxIndex(triId, 2)].xyz() } };
            if (cguMath::pointInTriangleTest<float>(tri, pt, nullptr)) return triId;
        }
        return -1;
    }*/

    std::tuple<std::unique_ptr<ConnectivitySubMesh>, bool> ConnectivitySubMesh::load(std::ifstream& ifs, const Mesh* mesh, const ConnectivityMesh* cmesh)
    {
        bool correctHeader;
        unsigned int actualVersion;
        std::tie(correctHeader, actualVersion) = VersionableSerializerType::checkHeader(ifs);
        if (correctHeader) {
            std::unique_ptr<ConnectivitySubMesh> result{ new ConnectivitySubMesh(mesh, cmesh) };
            serializeHelper::read(ifs, result->subMeshId_);
            serializeHelper::read(ifs, result->triangleRangeStart_);
            serializeHelper::read(ifs, result->numTriangles_);
            serializeHelper::read(ifs, result->aabb_.minmax[0]);
            serializeHelper::read(ifs, result->aabb_.minmax[1]);

            // result->CreateTriangleRTree();

            return std::make_tuple(std::move(result), true);
        }
        return std::make_tuple(nullptr, false);
    }

    void ConnectivitySubMesh::save(std::ofstream& ofs) const
    {
        VersionableSerializerType::writeHeader(ofs);
        serializeHelper::write(ofs, subMeshId_);
        serializeHelper::write(ofs, triangleRangeStart_);
        serializeHelper::write(ofs, numTriangles_);

        serializeHelper::write(ofs, aabb_.minmax[0]);
        serializeHelper::write(ofs, aabb_.minmax[1]);
    }

    /**
         *  Creates the sub meshes bounding box.
         */
    void ConnectivitySubMesh::CreateAABB()
    {
        auto& vertices = mesh_->GetVertices();
        if (numTriangles_ == 0) return;
        aabb_.minmax[0] = aabb_.minmax[1] = vertices[cMesh_->GetTriangle(triangleRangeStart_).vertex_[0]].xyz();
        for (const auto& tri : cMesh_->GetTriangles()) {
            for (auto vi : tri.locOnlyVtxIds_) {
                aabb_.minmax[0] = glm::min(aabb_.minmax[0], vertices[vi].xyz());
                aabb_.minmax[1] = glm::max(aabb_.minmax[1], vertices[vi].xyz());
            }
        }
    }
}
