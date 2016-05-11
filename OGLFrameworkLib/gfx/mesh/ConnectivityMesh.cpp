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

#undef min
#undef max


namespace cgu {

    /** Default constructor. */
    ConnectivityMesh::ConnectivityMesh(const Mesh* mesh) :
        mesh_(mesh)
    {
        std::vector<unsigned int> reducedVertexMap(mesh->GetVertices().size());
        for (auto i = 0; i < mesh->GetVertices().size(); ++i) {
            unsigned int currentReducedVertex = i;
            for (auto j = 0; j < i; ++j) {
                if (mesh->GetVertices()[i] == mesh->GetVertices()[j]) {
                    currentReducedVertex = j;
                    break;
                }
            }
            reducedVertexMap[i] = currentReducedVertex;
        }


        for (unsigned int i = 0; i < mesh_->GetNumSubmeshes(); ++i) {
            subMeshConnectivity_.emplace_back(std::make_unique<ConnectivitySubMesh>(mesh_, reducedVertexMap, i));
        }
        mesh_->GetRootNode()->GetBoundingBox(aabb_, mesh_->GetRootTransform());
    }

    /** Copy constructor. */
    ConnectivityMesh::ConnectivityMesh(const ConnectivityMesh& rhs) :
        mesh_(rhs.mesh_),
        aabb_(rhs.aabb_)
    {
        std::vector<unsigned int> reducedVertexMap(mesh_->GetVertices().size());
        for (auto i = 0; i < mesh_->GetVertices().size(); ++i) {
            unsigned int currentReducedVertex = i;
            for (auto j = 0; j < i; ++j) {
                if (mesh_->GetVertices()[i] == mesh_->GetVertices()[j]) {
                    currentReducedVertex = j;
                    break;
                }
            }
            reducedVertexMap[i] = currentReducedVertex;
        }

        for (unsigned int i = 0; i < mesh_->GetNumSubmeshes(); ++i) {
            subMeshConnectivity_.emplace_back(std::make_unique<ConnectivitySubMesh>(mesh_, reducedVertexMap, i));
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
        aabb_(std::move(rhs.aabb_)),
        subMeshConnectivity_(std::move(rhs.subMeshConnectivity_))
    {
    }

    /** Default move assignment operator. */
    ConnectivityMesh& ConnectivityMesh::operator=(ConnectivityMesh&& rhs)
    {
        if (this != &rhs) {
            this->~ConnectivityMesh();
            mesh_ = std::move(rhs.mesh_);
            aabb_ = std::move(rhs.aabb_);
            subMeshConnectivity_ = std::move(rhs.subMeshConnectivity_);
        }
        return *this;
    }

    /** Default destructor. */
    ConnectivityMesh::~ConnectivityMesh() = default;

    /**
     *  Find index of triangle that contains the given point.
     *  @param point the point to find the triangle for.
     */
    unsigned int ConnectivityMesh::FindContainingTriangle(const glm::vec3 point)
    {
        for (auto& submesh : subMeshConnectivity_) {
            auto result = submesh->FindContainingTriangle(point);
            if (result != -1) return result;
        }
        throw std::out_of_range("Containing triangle not found!");
    }
}
