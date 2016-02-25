/**
 * @file   SceneMeshNode.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.02.16
 *
 * @brief  Definition of a scene node containing mesh data.
 */

#ifndef SCENEMESHNODE_H
#define SCENEMESHNODE_H

#include "main.h"
#include <core/math/math.h>

struct aiNode;

namespace cgu {

    class SubMesh;

    class SceneMeshNode
    {
    public:
        SceneMeshNode() : nodeName_(""), parent_(nullptr) { aabb_.minmax[0] = glm::vec3(std::numeric_limits<float>::infinity()); aabb_.minmax[1] = glm::vec3(-std::numeric_limits<float>::infinity()); }
        SceneMeshNode(aiNode* node, const SceneMeshNode* parent, const std::vector<std::unique_ptr<SubMesh>>& meshes);
        SceneMeshNode(const SceneMeshNode& rhs);
        SceneMeshNode& operator=(const SceneMeshNode& rhs);
        SceneMeshNode(SceneMeshNode&& rhs);
        SceneMeshNode operator=(SceneMeshNode&& rhs);
        ~SceneMeshNode();

        void GetBoundingBox(cguMath::AABB3<float>& aabb, const glm::mat4& transform) const;
        glm::mat4 GetLocalTransform() const { return localTransform_; }
        unsigned int GetNumNodes() const { return static_cast<unsigned int>(children_.size()); }
        const SceneMeshNode* GetChild(unsigned int idx) const { return children_[idx].get(); }
        unsigned int GetNumMeshes() const { return static_cast<unsigned int>(meshes_.size()); }
        const SubMesh* GetMesh(unsigned int idx) const { return meshes_[idx]; }

        void write(std::ofstream& ofs);
        void read(std::ifstream& ifs, const std::unordered_map<uint64_t, SubMesh*>& meshes, std::unordered_map<uint64_t, SceneMeshNode*>& nodes);

    private:
        /** The nodes name. */
        std::string nodeName_;
        /** The nodes childen. */
        std::vector<std::unique_ptr<SceneMeshNode>> children_;
        /** The meshes in this node. */
        std::vector<SubMesh*> meshes_;
        /** The local transformation matrix. */
        glm::mat4 localTransform_;
        /** The nodes local AABB. */
        cguMath::AABB3<float> aabb_;
        /** The nodes parent. */
        const SceneMeshNode* parent_;
    };
}

#endif // SCENEMESHNODE_H
