/**
 * @file   SubMesh.cpp
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2014.01.13
 *
 * @brief  Implementation of the SubMesh.
 */

#define GLM_SWIZZLE

#include "SubMesh.h"
#include "core/serializationHelper.h"
#include "Mesh.h"
#include <unordered_map>
#include <fstream>

#undef min
#undef max

namespace cgu {

    /** Constructor. */
    SubMesh::SubMesh(const Mesh* mesh, const std::string& objectName, unsigned int indexOffset, unsigned int numIndices, Material* material) :
        objectName_(objectName),
        indexOffset_(indexOffset),
        numIndices_(numIndices),
        material_(material)
    {
        aabb_.minmax[0] = glm::vec3(std::numeric_limits<float>::infinity()); aabb_.minmax[1] = glm::vec3(-std::numeric_limits<float>::infinity());
        if (numIndices_ == 0) return;
        auto& vertices = mesh->GetVertices();
        auto& indices = mesh->GetIndices();
        aabb_.minmax[0] = aabb_.minmax[1] = vertices[indices[indexOffset_]].xyz();
        for (auto i = indexOffset_; i < indexOffset_ + numIndices_; ++i) {
            aabb_.minmax[0] = glm::min(aabb_.minmax[0], vertices[indices[i]].xyz());
            aabb_.minmax[1] = glm::max(aabb_.minmax[1], vertices[indices[i]].xyz());
        }
    }

    /** Default destructor. */
    SubMesh::~SubMesh() = default;
    /** Default copy constructor. */
    SubMesh::SubMesh(const SubMesh&) = default;
    /** Default copy assignment operator. */
    SubMesh& SubMesh::operator=(const SubMesh&) = default;

    /** Default move constructor. */
    SubMesh::SubMesh(SubMesh&& rhs) :
        objectName_(std::move(rhs.objectName_)),
        indexOffset_(std::move(rhs.indexOffset_)),
        numIndices_(std::move(rhs.numIndices_)),
        aabb_(std::move(rhs.aabb_)),
        material_(std::move(rhs.material_))
    {
    }

    /** Default move assignment operator. */
    SubMesh& SubMesh::operator=(SubMesh&& rhs)
    {
        if (this != &rhs) {
            this->~SubMesh();
            objectName_ = std::move(rhs.objectName_);
            indexOffset_ = std::move(rhs.indexOffset_);
            numIndices_ = std::move(rhs.numIndices_);
            aabb_ = std::move(rhs.aabb_);
            material_ = std::move(rhs.material_);
        }
        return *this;
    }

    void SubMesh::UpdateMaterials(const std::unordered_map<Material*, Material*>& materialUpdates)
    {
        material_ = materialUpdates.at(material_);
    }

    void SubMesh::write(std::ofstream& ofs) const
    {
        VersionableSerializerType::writeHeader(ofs);
        serializeHelper::write(ofs, reinterpret_cast<uint64_t>(this));
        serializeHelper::write(ofs, objectName_);
        serializeHelper::write(ofs, indexOffset_);
        serializeHelper::write(ofs, numIndices_);
        serializeHelper::write(ofs, aabb_.minmax[0]);
        serializeHelper::write(ofs, aabb_.minmax[1]);
        serializeHelper::write(ofs, reinterpret_cast<uint64_t>(material_));
    }

    bool SubMesh::read(std::ifstream& ifs, std::unordered_map<uint64_t, SubMesh*>& meshes, std::unordered_map<uint64_t, Material*>& materials)
    {
        bool correctHeader;
        unsigned int actualVersion;
        std::tie(correctHeader, actualVersion) = VersionableSerializerType::checkHeader(ifs);
        if (correctHeader) {
            uint64_t meshID, materialID;
            serializeHelper::read(ifs, meshID);
            serializeHelper::read(ifs, objectName_);
            serializeHelper::read(ifs, indexOffset_);
            serializeHelper::read(ifs, numIndices_);
            serializeHelper::read(ifs, aabb_.minmax[0]);
            serializeHelper::read(ifs, aabb_.minmax[1]);
            serializeHelper::read(ifs, materialID);
            meshes[meshID] = this;
            material_ = materials[materialID];
            return true;
        }
        return false;
    }
}
