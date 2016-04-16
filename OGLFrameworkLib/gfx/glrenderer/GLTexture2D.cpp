/**
 * @file   GLTexture2D.cpp
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2013.12.31
 *
 * @brief  Contains the implementation of GLTexture2D.
 */

#include "GLTexture2D.h"
#include "GLTexture.h"
#include <stb_image.h>
#include <boost/filesystem.hpp>
#include "app/ApplicationBase.h"
#include "app/Configuration.h"

namespace cgu {

    /**
     * Constructor.
     * @param texFilename the textures file name
     */
    GLTexture2D::GLTexture2D(const std::string& texFilename, ApplicationBase* app) :
        Resource{ texFilename, app },
        texture()
    {
        auto filename = FindResourceLocation(GetParameters()[0]);
        if (!boost::filesystem::exists(filename)) {
            LOG(ERROR) << "File \"" << filename.c_str() << L"\" cannot be opened.";
            throw resource_loading_error() << ::boost::errinfo_file_name(filename) << resid_info(getId())
                << errdesc_info("Cannot open texture file.");
        }

        stbi_set_flip_vertically_on_load(1);
        auto imgWidth = 0, imgHeight = 0, imgChannels = 0;
        auto image = stbi_load(filename.c_str(), &imgWidth, &imgHeight, &imgChannels, 0);
        if (!image) {
            LOG(ERROR) << L"Could not load texture \"" << filename.c_str() << L"\".";
            throw resource_loading_error() << ::boost::errinfo_file_name(filename) << resid_info(getId())
                << errdesc_info("Cannot load texture data.");
        }

        auto useSRGB = (CheckNamedParameterFlag("sRGB") && application->GetConfig().useSRGB);
        auto internalFmt = gl::GL_RGBA8;
        auto fmt = gl::GL_RGBA;
        switch (imgChannels) {
        case 1: internalFmt = gl::GL_R8; fmt = gl::GL_RED; break;
        case 2: internalFmt = gl::GL_RG8; fmt = gl::GL_RG; break;
        case 3: internalFmt = useSRGB ? gl::GL_SRGB8 : gl::GL_RGB8; fmt = gl::GL_RGB; break;
        case 4: internalFmt = useSRGB ? gl::GL_SRGB8_ALPHA8 : gl::GL_RGBA8; fmt = gl::GL_RGBA; break;
        default:
            LOG(ERROR) << L"Invalid number of texture channels (" << imgChannels << ").";
            throw resource_loading_error() << ::boost::errinfo_file_name(filename) << resid_info(getId())
                << errdesc_info("Invalid number of texture channels.");
        }

        TextureDescriptor texDesc(4, internalFmt, fmt, gl::GL_UNSIGNED_BYTE);
        texture = std::make_unique<GLTexture>(imgWidth, imgHeight, texDesc, image);
        if (CheckNamedParameterFlag("mirror")) texture->SampleWrapMirror();
        if (CheckNamedParameterFlag("repeat")) texture->SampleWrapRepeat();
        if (CheckNamedParameterFlag("clamp")) texture->SampleWrapClamp();
        if (CheckNamedParameterFlag("mirror-clamp")) texture->SampleWrapMirrorClamp();

        stbi_image_free(image);
    }

    /** Copy constructor. */
    GLTexture2D::GLTexture2D(const GLTexture2D& rhs) : GLTexture2D(rhs.getId(), rhs.application)
    {
    }

    /** Copy assignment operator. */
    GLTexture2D& GLTexture2D::operator=(const GLTexture2D& rhs)
    {
        auto tmp(rhs);
        std::swap(texture, tmp.texture);
        return *this;
    }

    /** Default move constructor. */
    GLTexture2D::GLTexture2D(GLTexture2D&& rhs) : Resource(std::move(rhs)), texture(std::move(rhs.texture)) {}

    /** Default move assignment operator. */
    GLTexture2D& GLTexture2D::operator=(GLTexture2D&& rhs)
    {
        if (this != &rhs) {
            this->~GLTexture2D();
            Resource* tRes = this;
            *tRes = static_cast<Resource&&>(std::move(rhs));
            texture = std::move(rhs.texture);
        }
        return *this;
    }

    /** Destructor. */
    GLTexture2D::~GLTexture2D() = default;

    /** Returns the texture object. */
    GLTexture* GLTexture2D::GetTexture()
    {
        return texture.get();
    }

    /** Returns the texture object. */
    const GLTexture* GLTexture2D::GetTexture() const
    {
        return texture.get();
    }
}
