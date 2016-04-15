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

namespace cgu {

    class Mesh;
    class ConnectivitySubMesh;

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

        unsigned int FindContainingTriangle(const glm::vec3 point);
        const std::vector<std::unique_ptr<ConnectivitySubMesh>>& GetSubMeshes() const { return subMeshConnectivity_; }

    private:
        /** The mesh to create connectivity from. */
        const Mesh* mesh_;
        /** Contains a bounding box containing all sub-meshes. */
        cguMath::AABB3<float> aabb_;
        /** Connectivity information for the sub-meshes. */
        std::vector<std::unique_ptr<ConnectivitySubMesh>> subMeshConnectivity_;
    };
}

#endif /* CONNECTIVITYMESH_H */
