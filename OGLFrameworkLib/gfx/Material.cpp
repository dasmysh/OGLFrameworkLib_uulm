/**
 * @file   Material.cpp
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2013.12.31
 *
 * @brief  Contains the implementation of Material methods.
 */

#include "Material.h"

namespace cgu {

    MaterialParameters::MaterialParameters() :
        diffuseAlbedo(0.0f, 0.0f, 0.0f),
        refraction(1.0f),
        specularScaling(0.0f, 0.0f, 0.0f),
        roughness(1.0f),
        specularExponent(1.0f)
    {
    }

    MaterialParameters::MaterialParameters(const MaterialParameters&) = default;
    MaterialParameters& MaterialParameters::operator=(const MaterialParameters&) = default;

    MaterialParameters::MaterialParameters(MaterialParameters&& rhs) :
        diffuseAlbedo(std::move(rhs.diffuseAlbedo)),
        refraction(std::move(rhs.refraction)),
        specularScaling(std::move(rhs.specularScaling)),
        roughness(std::move(rhs.roughness)),
        specularExponent(std::move(rhs.specularExponent))
    {
    }

    MaterialParameters& MaterialParameters::operator=(MaterialParameters&& rhs)
    {
        this->~MaterialParameters();
        diffuseAlbedo = std::move(rhs.diffuseAlbedo);
        refraction = std::move(rhs.refraction);
        specularScaling = std::move(rhs.specularScaling);
        roughness = std::move(rhs.roughness);
        specularExponent = std::move(rhs.specularExponent);
        return *this;
    }

    MaterialParameters::~MaterialParameters() = default;

    Material::Material() :
        ambient(0.0f, 0.0f, 0.0f),
        alpha(1.0f),
        minOrientedAlpha(0.0f),
        diffuseTex(nullptr),
        bumpTex(nullptr),
        bumpMultiplier(1.0f)
    {
    }

    /** Default copy constructor. */
    Material::Material(const Material&) = default;
    /** Default copy assignment operator. */
    Material& Material::operator=(const Material&) = default;

    /** Default move constructor. */
    Material::Material(Material&& rhs) :
        params(std::move(rhs.params)),
        ambient(std::move(rhs.ambient)),
        alpha(std::move(rhs.alpha)),
        minOrientedAlpha(std::move(rhs.minOrientedAlpha)),
        diffuseTex(std::move(rhs.diffuseTex)),
        bumpTex(std::move(rhs.bumpTex)),
        bumpMultiplier(std::move(rhs.bumpMultiplier))
    {
    }

    /** Default move assignment operator. */
    Material& Material::operator=(Material&& rhs)
    {
        this->~Material();
        ambient = std::move(rhs.ambient);
        alpha = std::move(rhs.alpha);
        minOrientedAlpha = std::move(rhs.minOrientedAlpha);
        diffuseTex = std::move(rhs.diffuseTex);
        bumpTex = std::move(rhs.bumpTex);
        bumpMultiplier = std::move(rhs.bumpMultiplier);
        return *this;
    }

    Material::~Material() = default;
}
