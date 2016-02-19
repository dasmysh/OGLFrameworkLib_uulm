/**
 * @file   SceneMeshNode.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.02.16
 *
 * @brief  Implementation of a scene node containing mesh data.
 */

#include "SceneMeshNode.h"
#include <assimp/scene.h>
#include "core/serializationHelper.h"
#include "SubMesh.h"
#include <core/math/transforms.h>


namespace cgu {

    static void CopyAiMatrixToGLM(const aiMatrix4x4& from, glm::mat4 &to)
    {
        to[0][0] = from.a1; to[1][0] = from.a2;
        to[2][0] = from.a3; to[3][0] = from.a4;
        to[0][1] = from.b1; to[1][1] = from.b2;
        to[2][1] = from.b3; to[3][1] = from.b4;
        to[0][2] = from.c1; to[1][2] = from.c2;
        to[2][2] = from.c3; to[3][2] = from.c4;
        to[0][3] = from.d1; to[1][3] = from.d2;
        to[2][3] = from.d3; to[3][3] = from.d4;
    }

    SceneMeshNode::SceneMeshNode(aiNode* node, const SceneMeshNode* parent, const std::vector<std::unique_ptr<SubMesh>>& meshes) :
        nodeName_(node->mName.C_Str()),
        parent_(parent)
    {
        aabb_.minmax[0] = glm::vec3(std::numeric_limits<float>::infinity()); aabb_.minmax[1] = glm::vec3(-std::numeric_limits<float>::infinity());
        CopyAiMatrixToGLM(node->mTransformation, localTransform_);
        for (unsigned int i = 0; i < node->mNumMeshes; ++i) meshes_.push_back(meshes[i].get());
        for (unsigned int i = 0; i < node->mNumChildren; ++i) children_.push_back(std::make_unique<SceneMeshNode>(node->mChildren[i], this, meshes));

        for (const auto& mesh : meshes_) {
            cguMath::AABB3<float> meshAABB = cguMath::transformAABB(mesh->GetLocalAABB(), localTransform_);
            aabb_.minmax[0] = glm::min(aabb_.minmax[0], meshAABB.minmax[0]);
            aabb_.minmax[1] = glm::max(aabb_.minmax[1], meshAABB.minmax[1]);
        }

        for (const auto& child : children_) {
            cguMath::AABB3<float> nodeAABB;
            child->GetBoundingBox(nodeAABB, localTransform_);
            aabb_.minmax[0] = glm::min(aabb_.minmax[0], nodeAABB.minmax[0]);
            aabb_.minmax[1] = glm::max(aabb_.minmax[1], nodeAABB.minmax[1]);
        }
    }

    SceneMeshNode::SceneMeshNode(const SceneMeshNode& rhs) :
        nodeName_(rhs.nodeName_),
        meshes_(rhs.meshes_),
        localTransform_(rhs.localTransform_),
        parent_(rhs.parent_)
    {
        children_.resize(rhs.children_.size());
        for (auto i = 0; i < children_.size(); ++i) children_[i] = std::make_unique<SceneMeshNode>(*rhs.children_[i]);
    }

    SceneMeshNode::SceneMeshNode(SceneMeshNode&& rhs) :
        nodeName_(std::move(rhs.nodeName_)),
        children_(std::move(rhs.children_)),
        meshes_(std::move(rhs.meshes_)),
        localTransform_(std::move(rhs.localTransform_)),
        parent_(std::move(rhs.parent_))
    {

    }

    SceneMeshNode& SceneMeshNode::operator=(const SceneMeshNode& rhs)
    {
        if (this != &rhs) {
            SceneMeshNode tmp{ rhs };
            std::swap(*this, tmp);
        }
        return *this;
    }

    SceneMeshNode SceneMeshNode::operator=(SceneMeshNode&& rhs)
    {
        if (this != &rhs) {
            this->~SceneMeshNode();
            nodeName_ = std::move(rhs.nodeName_);
            children_ = std::move(rhs.children_);
            meshes_ = std::move(rhs.meshes_);
            localTransform_ = std::move(rhs.localTransform_);
            parent_ = std::move(rhs.parent_);
        }
        return *this;
    }

    SceneMeshNode::~SceneMeshNode() = default;

    void SceneMeshNode::GetBoundingBox(cguMath::AABB3<float>& aabb, const glm::mat4& transform) const
    {
        aabb = cguMath::transformAABB(aabb_, transform);
    }

    void SceneMeshNode::write(std::ofstream& ofs)
    {
        std::vector<size_t> meshWriteIds(meshes_.size());
        for (auto i = 0; i < meshes_.size(); ++i) meshWriteIds[i] = reinterpret_cast<uint64_t>(meshes_[i]);

        std::vector<size_t> nodeWriteIds(children_.size());
        for (auto i = 0; i < children_.size(); ++i) nodeWriteIds[i] = reinterpret_cast<uint64_t>(children_[i].get());

        serializeHelper::write(ofs, reinterpret_cast<uint64_t>(this));
        serializeHelper::write(ofs, nodeName_);
        serializeHelper::write(ofs, localTransform_);
        serializeHelper::write(ofs, aabb_.minmax[0]);
        serializeHelper::write(ofs, aabb_.minmax[1]);
        serializeHelper::write(ofs, localTransform_);
        serializeHelper::write(ofs, reinterpret_cast<uint64_t>(parent_));
        serializeHelper::writeV(ofs, meshWriteIds);
        serializeHelper::writeV(ofs, nodeWriteIds);
        for (auto i = 0; i < children_.size(); ++i) children_[i]->write(ofs);
    }

    void SceneMeshNode::read(std::ifstream& ifs, const std::unordered_map<uint64_t, SubMesh*>& meshes, std::unordered_map<uint64_t, SceneMeshNode*>& nodes)
    {
        children_.clear();
        meshes_.clear();

        uint64_t nodeID, parentNodeID;
        std::vector<uint64_t> meshIDs, childIDs;

        serializeHelper::read(ifs, nodeID);
        serializeHelper::read(ifs, nodeName_);
        serializeHelper::read(ifs, localTransform_);
        serializeHelper::read(ifs, aabb_.minmax[0]);
        serializeHelper::read(ifs, aabb_.minmax[1]);
        serializeHelper::read(ifs, parentNodeID);
        serializeHelper::readV(ifs, meshIDs);
        serializeHelper::readV(ifs, childIDs);

        meshes_.resize(meshIDs.size());
        for (auto i = 0; i < meshes_.size(); ++i) meshes_[i] = meshes.at(meshIDs[i]);

        parent_ = nodes[parentNodeID];
        nodes[nodeID] = this;

        children_.resize(childIDs.size());
        for (auto i = 0; i < children_.size(); ++i) {
            children_[i] = std::make_unique<SceneMeshNode>();
            children_[i]->read(ifs, meshes, nodes);
        }
    }

}
