/**
 * @file   GLTexture2D.h
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2013.12.31
 *
 * @brief  Contains texture class for openGL implementation.
 */

#ifndef GLTEXTURE2D_H
#define GLTEXTURE2D_H

#include "main.h"
#include "core/Resource.h"

namespace cgu {
    class GLTexture;

    /**
     * @brief  2D Texture for the OpenGL implementation.
     *
     * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
     * @date   2013.12.31
     */
    class GLTexture2D : public Resource
    {
    public:
        GLTexture2D(const std::string& texFilename, ApplicationBase* app);
        GLTexture2D(const GLTexture2D&);
        GLTexture2D& operator=(const GLTexture2D&);
        GLTexture2D(GLTexture2D&&);
        GLTexture2D& operator=(GLTexture2D&&);
        virtual ~GLTexture2D();

        GLTexture* GetTexture();
        const GLTexture* GetTexture() const;

    private:
        void LoadTextureLDR(const std::string& filename);
        void LoadTextureHDR(const std::string& filename);
        std::tuple<int, int> FindFormat(const std::string& filename, int imgChannels) const;

        /** Holds the texture. */
        std::unique_ptr<GLTexture> texture_;
    };
}

#endif /* GLTEXTURE2D_H */
