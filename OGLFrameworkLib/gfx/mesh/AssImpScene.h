/**
 * @file   AssimpScene.h
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2016.02.16
 *
 * @brief  Contains the definition of a scene loaded by assimp.
 */

#ifndef ASSIMPSCENE_H
#define ASSIMPSCENE_H

#include "main.h"
#include "gfx/Material.h"
#include "Mesh.h"
#include <core/serializationHelper.h>

namespace cgu {

    /**
     * @brief  Resource implementation for .obj files.
     * This is used to generate renderable meshes from .obj files.
     *
     * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
     * @date   2014.01.08
     */
    class AssimpScene : public Resource, public Mesh
    {
    public:
        AssimpScene(const std::string& objFilename, ApplicationBase* app);
        AssimpScene(const AssimpScene&);
        AssimpScene& operator=(const AssimpScene&);
        AssimpScene(AssimpScene&&);
        AssimpScene& operator=(AssimpScene&&);
        virtual ~AssimpScene();

        std::string GetFullFilename() const override;

    private:
        using VersionableSerializerType = serializeHelper::VersionableSerializer<'M', 'B', 'A', 'M', 1001>;

        void createNewMesh(const std::string& filename, const std::string& textureParamsString, const std::string& binFilename, ApplicationBase* app);
        std::shared_ptr<const GLTexture2D> loadTexture(const std::string& relFilename, const std::string& params, ApplicationBase* app) const;
        void save(const std::string& filename) const;
        bool load(const std::string& filename, ApplicationBase* app);
    };
}

#endif /* ASSIMPSCENE_H */
