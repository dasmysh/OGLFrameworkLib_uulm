/**
 * @file   MaterialLibrary.h
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2014.01.04
 *
 * @brief  Contains definition of MaterialLibrary.
 */

#ifndef MATERIALLIBRARY_H
#define MATERIALLIBRARY_H

#include <boost/regex.hpp>

#include "main.h"
#include "Material.h"

namespace cgu {

    class GLTexture2D;

    struct MaterialResourceLoadingPolicy
    {
        static std::shared_ptr<Material> CreateResource(const std::string&, ApplicationBase*)
        {
            return std::move(std::make_shared<Material>());
        }

        static bool IsResourceLoaded(const Material*) { return true; }
        static void LoadResource(Material*) {}
    };

    /**
     * @brief  Loads material libraries and manages its materials.
     *
     * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
     * @date   2014.01.04
     */
    class MaterialLibrary final : public Resource, public ResourceManager<Material, false, MaterialResourceLoadingPolicy>
    {
    public:
        MaterialLibrary(const std::string& mtlFilename, ApplicationBase* app);
        MaterialLibrary(const MaterialLibrary&);
        MaterialLibrary& operator=(const MaterialLibrary&);
        MaterialLibrary(MaterialLibrary&&);
        MaterialLibrary& operator=(MaterialLibrary&&);
        virtual ~MaterialLibrary();

    private:
        glm::vec3 parseColor(const boost::smatch& matches) const;
        std::shared_ptr<const GLTexture2D> parseTexture(const std::string& matches, const std::string& params) const;
        float parseFloatParameter(const std::string& paramName, const std::string& matches, float defaultValue) const;
        static void notImplemented(const std::string& feature);
    };
}

#endif /* MATERIALLIBRARY_H */
