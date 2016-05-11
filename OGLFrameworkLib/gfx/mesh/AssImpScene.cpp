/**
 * @file   AssimpScene.cpp
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2016.02.16
 *
 * @brief  Contains the implementation of a scene loaded by assimp.
 */

#define GLM_SWIZZLE
#include "AssimpScene.h"
#include "core/glm_helper.h"
#include "app/ApplicationBase.h"
#include <fstream>
#include <boost/filesystem.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <unordered_map>
#include <glm/gtx/transform.hpp>
#include <boost/algorithm/string/join.hpp>

namespace cgu {

    struct model_binload_exception : virtual boost::exception, virtual std::exception { };

    /**
     * Constructor.
     * @param objFilename the .obj files file name.
     * @param mtlLibManager the material library manager to use
     */
    AssimpScene::AssimpScene(const std::string& objFilename, ApplicationBase* app) :
        Resource(objFilename, app)
    {
        auto filename = FindResourceLocation(GetParameter(0));
        auto binFilename = filename + ".myshbin";
        std::vector<std::string> textureParams;
        if (CheckNamedParameterFlag("textureRepeat")) textureParams.push_back("-repeat");
        if (CheckNamedParameterFlag("textureMirror")) textureParams.push_back("-mirror");
        if (CheckNamedParameterFlag("textureClamp")) textureParams.push_back("-clamp");
        if (CheckNamedParameterFlag("textureMirrorClamp")) textureParams.push_back("-mirror-clamp");

        auto textureParamsString = boost::algorithm::join(textureParams, ",");
        if (boost::filesystem::exists(binFilename)) {
            try {
                load(binFilename, app);
            }
            catch (model_binload_exception) {
            }
        } else {
            unsigned int assimpFlags = aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_LimitBoneWeights
                | aiProcess_ImproveCacheLocality | aiProcess_RemoveRedundantMaterials | aiProcess_OptimizeMeshes
                | aiProcess_OptimizeGraph;
            if (CheckNamedParameterFlag("createTangents")) assimpFlags |= aiProcess_CalcTangentSpace;
            if (CheckNamedParameterFlag("noSmoothNormals")) assimpFlags |= aiProcess_GenNormals;
            else assimpFlags |= aiProcess_GenSmoothNormals;

            Assimp::Importer importer;
            auto scene = importer.ReadFile(filename, assimpFlags);

            unsigned int maxUVChannels = 0, maxColorChannels = 0, numVertices = 0, numIndices = 0;
            std::vector<std::vector<unsigned int>> indices;
            indices.resize(scene->mNumMeshes);
            for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
                maxUVChannels = glm::max(maxUVChannels, scene->mMeshes[i]->GetNumUVChannels());
                maxColorChannels = glm::max(maxColorChannels, scene->mMeshes[i]->GetNumUVChannels());
                numVertices += scene->mMeshes[i]->mNumVertices;
                for (unsigned int fi = 0; fi < scene->mMeshes[i]->mNumFaces; ++fi) {
                    auto faceIndices = scene->mMeshes[i]->mFaces[fi].mNumIndices;
                    if (faceIndices == 3) {
                        indices[i].push_back(scene->mMeshes[i]->mFaces[fi].mIndices[0]);
                        indices[i].push_back(scene->mMeshes[i]->mFaces[fi].mIndices[1]);
                        indices[i].push_back(scene->mMeshes[i]->mFaces[fi].mIndices[2]);
                        numIndices += faceIndices;
                    } else {
                        // TODO: handle points and lines. [2/17/2016 Sebastian Maisch]
                    }
                }
            }

            ReserveMesh(maxUVChannels, maxColorChannels, numVertices, numIndices, scene->mNumMaterials);
            for (unsigned int i = 0; i < scene->mNumMaterials; ++i) {
                auto material = scene->mMaterials[i];
                auto mat = GetMaterial(i);
                material->Get(AI_MATKEY_COLOR_AMBIENT, mat->ambient);
                material->Get(AI_MATKEY_COLOR_DIFFUSE, mat->params.diffuseAlbedo);
                material->Get(AI_MATKEY_COLOR_SPECULAR, mat->params.specularScaling);
                material->Get(AI_MATKEY_OPACITY, mat->alpha);
                material->Get(AI_MATKEY_SHININESS, mat->params.specularExponent);
                material->Get(AI_MATKEY_REFRACTI, mat->params.refraction);
                aiString diffuseTexPath, bumpTexPath;
                if (AI_SUCCESS == material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), diffuseTexPath)) {
                    mat->diffuseTex = loadTexture(diffuseTexPath.C_Str(), textureParamsString + ",-sRGB", app);
                }

                if (AI_SUCCESS == material->Get(AI_MATKEY_TEXTURE(aiTextureType_HEIGHT, 0), bumpTexPath)) {
                    mat->bumpTex = loadTexture(bumpTexPath.C_Str(), textureParamsString, app);
                    material->Get(AI_MATKEY_TEXBLEND(aiTextureType_HEIGHT, 0), mat->bumpMultiplier);
                } else if (AI_SUCCESS == material->Get(AI_MATKEY_TEXTURE(aiTextureType_NORMALS, 0), bumpTexPath)) {
                    mat->bumpTex = loadTexture(diffuseTexPath.C_Str(), textureParamsString, app);
                    material->Get(AI_MATKEY_TEXBLEND(aiTextureType_NORMALS, 0), mat->bumpMultiplier);
                }
            }

            unsigned int currentMeshIndexOffset = 0;
            unsigned int currentMeshVertexOffset = 0;
            for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
                auto mesh = scene->mMeshes[i];

                if (mesh->HasPositions()) {
                    std::copy(mesh->mVertices, &mesh->mVertices[mesh->mNumVertices], reinterpret_cast<aiVector3D*>(&GetVertices()[currentMeshVertexOffset]));
                }
                if (mesh->HasNormals()) {
                    std::copy(mesh->mNormals, &mesh->mNormals[mesh->mNumVertices], reinterpret_cast<aiVector3D*>(&GetNormals()[currentMeshVertexOffset]));
                }
                for (unsigned int ti = 0; ti < mesh->GetNumUVChannels(); ++ti) {
                    std::copy(mesh->mTextureCoords[ti], &mesh->mTextureCoords[ti][mesh->mNumVertices], reinterpret_cast<aiVector3D*>(&GetTexCoords()[ti][currentMeshVertexOffset]));
                }
                if (mesh->HasTangentsAndBitangents()) {
                    std::copy(mesh->mTangents, &mesh->mTangents[mesh->mNumVertices], reinterpret_cast<aiVector3D*>(&GetTangents()[currentMeshVertexOffset]));
                    std::copy(mesh->mBitangents, &mesh->mBitangents[mesh->mNumVertices], reinterpret_cast<aiVector3D*>(&GetBinormals()[currentMeshVertexOffset]));
                }
                for (unsigned int ci = 0; ci < mesh->GetNumColorChannels(); ++ci) {
                    std::copy(mesh->mColors[ci], &mesh->mColors[ci][mesh->mNumVertices], reinterpret_cast<aiColor4D*>(&GetColors()[ci][currentMeshVertexOffset]));
                }

                std::transform(indices[i].begin(), indices[i].end(), &GetIndices()[currentMeshIndexOffset], [currentMeshVertexOffset](unsigned int idx){ return idx + currentMeshVertexOffset; });

                auto material = GetMaterial(mesh->mMaterialIndex);

                AddSubMesh(mesh->mName.C_Str(), currentMeshIndexOffset, static_cast<unsigned int>(indices[i].size()), material);
                currentMeshVertexOffset += mesh->mNumVertices;
                currentMeshIndexOffset += static_cast<unsigned int>(indices[i].size());
            }

            CreateSceneNodes(scene->mRootNode);
            save(binFilename);
        }

        CreateIndexBuffer();
        auto rootScale = GetNamedParameterValue("scale", 1.0f);
        auto rootScaleV = GetNamedParameterValue("scaleV", glm::vec3(1.0f));
        auto rootTranslate = GetNamedParameterValue("translate", glm::vec3(0.0f));
        auto matScale = glm::scale(rootScaleV * rootScale);
        auto matTranslate = glm::translate(rootTranslate);
        SetRootTransform(matTranslate * matScale);
    }

    /** Default copy constructor. */
    AssimpScene::AssimpScene(const AssimpScene& rhs) : Resource(rhs), Mesh(rhs) {}

    /** Default copy assignment operator. */
    AssimpScene& AssimpScene::operator=(const AssimpScene& rhs)
    {
        if (this != &rhs) {
            AssimpScene tmp{ rhs };
            std::swap(*this, tmp);
        }
        return *this;
    }

    /** Default move constructor. */
    AssimpScene::AssimpScene(AssimpScene&& rhs) : Resource(std::move(rhs)), Mesh(std::move(rhs)) {}

    /** Default move assignment operator. */
    AssimpScene& AssimpScene::operator=(AssimpScene&& rhs)
    {
        if (this != &rhs) {
            this->~AssimpScene();
            Resource* tRes = this;
            *tRes = static_cast<Resource&&>(std::move(rhs));
            Mesh* tMesh = this;
            *tMesh = static_cast<Mesh&&>(std::move(rhs));
        }
        return *this;
    }

    /** Destructor. */
    AssimpScene::~AssimpScene() = default;

    std::shared_ptr<const GLTexture2D> AssimpScene::loadTexture(const std::string& relFilename, const std::string& params, ApplicationBase* app) const
    {
        boost::filesystem::path sceneFilePath{ GetParameters()[0] };
        auto texFilename = sceneFilePath.parent_path().string() + "/" + relFilename + (params.size() > 0 ? "," + params : "");
        return std::move(app->GetTextureManager()->GetResource(texFilename));
    }

    void AssimpScene::save(const std::string& filename) const
    {
        std::ofstream ofs(filename, std::ios::out | std::ios::binary);
        Mesh::write(ofs);
    }

    void AssimpScene::load(const std::string& filename, ApplicationBase* app)
    {
        std::vector<std::string> params;

        std::ifstream inBinFile(filename, std::ios::binary);
        if (inBinFile.is_open()) {
            Mesh::read(inBinFile, *app->GetTextureManager());
        }
    }
}
