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
#include "ConnectivityMeshImpl.h"

#undef min
#undef max


namespace cgu {

    /** Default constructor. */
    ConnectivityMesh::ConnectivityMesh(const Mesh* mesh) :
        impl_(std::make_unique<impl::ConnectivityMeshImpl>(mesh))
    {
    }

    /** Copy constructor. */
    ConnectivityMesh::ConnectivityMesh(const ConnectivityMesh& rhs) :
        impl_(std::make_unique<impl::ConnectivityMeshImpl>(*impl_))
    {
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
        impl_(std::move(rhs.impl_))
    {
    }

    /** Default move assignment operator. */
    ConnectivityMesh& ConnectivityMesh::operator=(ConnectivityMesh&& rhs)
    {
        if (this != &rhs) {
            impl_ = std::move(rhs.impl_);
        }
        return *this;
    }

    /** Default destructor. */
    ConnectivityMesh::~ConnectivityMesh() = default;

    void ConnectivityMesh::FindPointsWithinRadius(const glm::vec3 center, float radius, std::vector<unsigned>& result) const
    {
        impl_->FindPointsWithinRadius(center, radius, result);
    }

    unsigned ConnectivityMesh::FindNearest(const glm::vec3 center) const
    {
        return impl_->FindNearest(center);
    }

    void ConnectivityMesh::FindTrianglesWithinRadius(const glm::vec3 center, float radius, std::vector<unsigned>& result) const
    {
        impl_->FindTrianglesWithinRadius(center, radius, result);
    }

    unsigned ConnectivityMesh::FindNearestTriangle(const glm::vec3 center) const
    {
        return impl_->FindNearestTriangle(center);
    }

    /**
     *  Find index of triangle that contains the given point.
     *  @param point the point to find the triangle for.
     */
    unsigned int ConnectivityMesh::FindContainingTriangle(const glm::vec3 pt) const
    {
        return impl_->FindContainingTriangle(pt);
    }

    const std::vector<std::unique_ptr<ConnectivitySubMesh>>& ConnectivityMesh::GetSubMeshes() const
    {
        return impl_->GetSubMeshes();
    }

    std::vector<size_t> ConnectivityMesh::GetAdjacentVertices(size_t vtxId) const
    {
        return impl_->GetAdjacentVertices(vtxId);
    }

    const std::vector<MeshConnectVertex>& ConnectivityMesh::GetVertices() const
    {
        return impl_->GetVertices();
    }

    const std::vector<MeshConnectTriangle>& ConnectivityMesh::GetTriangles() const
    {
        return impl_->GetTriangles();
    }

    const MeshConnectTriangle& ConnectivityMesh::GetTriangle(unsigned idx) const
    {
        return impl_->GetTriangle(idx);
    }

    /*bool ConnectivityMesh::loadV2(std::ifstream& inBinFile)
    {
        serializeHelper::readV(inBinFile, treeVertices_);
        if (!loadV1(inBinFile)) return false;

        serializeHelper::readV(inBinFile, treeTriangles_);
        CreateTriangleRTree(false);

        return true;
    }*/
}
