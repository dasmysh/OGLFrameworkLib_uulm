/**
 * @file   FrameBuffer.h
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2014.01.17
 *
 * @brief  Contains the definition of FrameBuffer.
 */

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <vector>
#include "main.h"
#include "GLTexture.h"

namespace cgu {

    /** Describes a render buffer. */
    struct RenderBufferDescriptor
    {
        /** Holds the internal format of the render buffer. */
        GLenum internalFormat_;
    };

    /** Describes a texture render target for frame buffers. */
    struct FrameBufferTextureDescriptor
    {
        // ReSharper disable once CppNonExplicitConvertingConstructor
        FrameBufferTextureDescriptor(TextureDescriptor texDesc, GLenum texType = GL_TEXTURE_2D) : texDesc_{texDesc}, texType_{ texType } {}

        /** The texture descriptor. */
        TextureDescriptor texDesc_;
        /** The texture type. */
        GLenum texType_;
    };

    /** Describes a frame buffer. */
    struct FrameBufferDescriptor
    {
        FrameBufferDescriptor() = default;
        FrameBufferDescriptor(const std::vector<FrameBufferTextureDescriptor>& tex, const std::vector<RenderBufferDescriptor>& rb) : texDesc_(tex), rbDesc_(rb), numSamples_(1) {}

        /** Holds descriptions for all textures used. */
        std::vector<FrameBufferTextureDescriptor> texDesc_;
        /** Holds descriptions for all render buffers used. */
        std::vector<RenderBufferDescriptor> rbDesc_;
        /** Holds the number of samples for the frame buffer. */
        unsigned int numSamples_ = 1;
    };

    /**
     * @brief  Represents frame buffers.
     *
     * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
     * @date   2014.01.17
     */
    class FrameBuffer
    {
    public:
        FrameBuffer();
        FrameBuffer(unsigned int fbWidth, unsigned int fbHeight, const FrameBufferDescriptor& desc);
        FrameBuffer(const FrameBuffer&);
        FrameBuffer(FrameBuffer&&);
        FrameBuffer& operator=(const FrameBuffer&);
        FrameBuffer& operator=(FrameBuffer&&);
        ~FrameBuffer();

        void UseAsRenderTarget();
        void UseAsRenderTarget(const std::vector<unsigned int> drawBufferIndices);
        void Resize(unsigned int fbWidth, unsigned int fbHeight);
        const std::vector<std::unique_ptr<GLTexture>>& GetTextures() const { return textures; };
        void ResolveFramebuffer(FrameBuffer* fb, unsigned int readBufferIndex, unsigned int drawBufferIndex) const;

        unsigned int GetWidth() const { return width; };
        unsigned int GetHeight() const { return height; };

    private:
        static unsigned int findAttachment(GLenum internalFormat, unsigned int& colorAtt, std::vector<GLenum> &drawBuffers);

        /** holds the frame buffers OpenGL name. */
        FramebufferRAII fbo;
        /** Holds whether this represents the back buffer or not. */
        bool isBackbuffer;
        /** holds the frame buffer type. */
        FrameBufferDescriptor desc;
        /** holds the frame buffers textures to render to. */
        std::vector<std::unique_ptr<GLTexture>> textures;
        /** Holds the a list of color attachments for the textures. */
        std::vector<GLenum> drawBuffers;
        /** holds the frame buffers render buffers to render to. */
        std::vector<RenderbufferRAII> renderBuffers;
        /** holds the frame buffers width. */
        unsigned int width;
        /** holds the frame buffers height. */
        unsigned int height;
    };
}
#endif /* FRAMEBUFFER_H */
