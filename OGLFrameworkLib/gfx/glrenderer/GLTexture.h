/**
 * @file   GLTexture.h
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2014.02.08
 *
 * @brief  Contains the definition of GLTexture.
 */

#ifndef GLTEXTURE_H
#define GLTEXTURE_H

#include "main.h"
#include "GPUProgram.h"

namespace cgu {
    class GLTexture;
    class FrameBuffer;

    namespace gpgpu {
        class CUDAImage;
    }

    /** Describes the format of a texture. */
    struct TextureDescriptor
    {
        TextureDescriptor(unsigned int btsPP, gl::GLenum intFmt, gl::GLenum fmt, gl::GLenum tp) : bytesPP(btsPP), internalFormat(intFmt), format(fmt), type(tp) {};

        /** Holds the bytes per pixel of the format. */
        unsigned int bytesPP;
        /** Holds the internal format. */
        gl::GLenum internalFormat;
        /** Holds the format. */
        gl::GLenum format;
        /** Holds the type. */
        gl::GLenum type;
    };

    class TextureGLIdentifierAccessor
    {
        friend class gpgpu::CUDAImage;
        friend class GLTexture;
        friend class FrameBuffer;
        friend class ShadowMap;

        TextureGLIdentifierAccessor(TextureRAII id, gl::GLenum type) : textureId(std::move(id)), textureType(type) {};
        explicit TextureGLIdentifierAccessor(gl::GLenum type) : textureType(type) {};
        TextureRAII textureId;
        gl::GLenum textureType;
    };

    /**
    * @brief  General representation of an OpenGL texture.
    *
    * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
    * @date   2014.02.08
    */
    class GLTexture
    {
        /** Deleted copy constructor. */
        GLTexture(const GLTexture&) = delete;
        /** Deleted copy assignment operator. */
        GLTexture& operator=(const GLTexture&)  = delete;

    public:
        GLTexture(TextureRAII texID, gl::GLenum texType, const TextureDescriptor& desc);
        GLTexture(unsigned int size, const TextureDescriptor& desc);
        GLTexture(unsigned int width, unsigned int height, unsigned int arraySize, const TextureDescriptor& desc);
        GLTexture(unsigned int width, unsigned int height, const TextureDescriptor& desc, const void* data);
        GLTexture(unsigned int width, unsigned int height, unsigned int depth, unsigned int numMipLevels, const TextureDescriptor& desc, const void* data);
        virtual ~GLTexture();

        void ActivateTexture(gl::GLenum textureUnit) const;
        void ActivateImage(gl::GLuint imageUnitIndex, gl::GLint mipLevel, gl::GLenum accessType) const;
        void AddTextureToArray(const std::string& file, unsigned int slice) const;
        void SetData(const void* data) const;
        void DownloadData(std::vector<uint8_t>& data) const;
        void UploadData(std::vector<uint8_t>& data) const;
        void GenerateMipMaps() const;
        void GenerateMinMaxMaps(GPUProgram* minMaxProgram, const std::vector<BindingLocation>& uniformNames) const;
        void ClearTexture(unsigned int mipLevel, const glm::vec4& data) const;
        glm::uvec3 GetDimensions() const { return glm::uvec3(width, height, depth); }
        glm::uvec3 GetLevelDimensions(int level) const;
        const TextureDescriptor& GetDescriptor() const { return descriptor; }

        void SampleWrapMirror() const;
        void SampleWrapClamp() const;
        void SampleWrapRepeat() const;
        void SampleWrapMirrorClamp() const;
        void SampleWrapBorderColor(const glm::vec4& color) const;
        void SampleLinear() const;
        void SampleNearest() const;

        void ActivateShadowMapComparison() const;

        const TextureGLIdentifierAccessor& GetGLIdentifier() const { return id; };

    private:
        void SetSampleWrap(gl::GLenum param) const;

        /** Holds the OpenGL texture id. */
        TextureGLIdentifierAccessor id;
        /** Holds the texture descriptor. */
        TextureDescriptor descriptor;

        /** Holds the width. */
        unsigned int width;
        /** Holds the height. */
        unsigned int height;
        /** Holds the depth or number of array slices. */
        unsigned int depth;
        /** Holds the number of MipMap levels the texture has. */
        unsigned int mipMapLevels;

        void InitSampling() const;
    };
}

#endif /* GLTEXTURE_H */
