/**
 * @file   FrameBuffer.cpp
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2014.01.17
 *
 * @brief  Contains the implementation of FrameBuffer.
 */

#include "FrameBuffer.h"
#include <exception>
#include <stdexcept>
#include "../../main.h"

namespace cgu {
    /**
     * Constructor.
     * Creates a FrameBuffer representing the backbuffer.
     */
    FrameBuffer::FrameBuffer() :
        fbo(0),
        isBackbuffer(true),
        desc(),
        textures(),
        renderBuffers(),
        width(0),
        height(0)
    {
    }

    /**
     * Constructor.
     * Creates a new FrameBuffer with given width and height. It is initialized as backbuffer as default.
     * @param fbWidth the frame buffers width
     * @param fbHeight the frame buffers height.
     * @param d the frame buffers description.
     */
    FrameBuffer::FrameBuffer(unsigned int fbWidth, unsigned int fbHeight, const FrameBufferDescriptor& d) :
        isBackbuffer(false),
        desc(d),
        width(0),
        height(0)
    {
        for (auto& texDesc : desc.texDesc_) {
            if (texDesc.texType_ == GL_TEXTURE_2D && d.numSamples_ != 1) texDesc.texType_ = GL_TEXTURE_2D_MULTISAMPLE;
        }
        Resize(fbWidth, fbHeight);
    }

    /**
     *  Copy constructor for a frame buffer.
     *  @param orig the original frame buffer.
     */
    FrameBuffer::FrameBuffer(const FrameBuffer& orig) :
        isBackbuffer(false),
        desc(orig.desc),
        width(0),
        height(0)
    {
        Resize(orig.width, orig.height);
    }

    /**
     *  Move-Constructs a frame buffer object.
     *  @param orig the original frame buffer.
     */
    FrameBuffer::FrameBuffer(FrameBuffer&& orig) :
        fbo(std::move(orig.fbo)),
        isBackbuffer(orig.isBackbuffer),
        desc(std::move(orig.desc)),
        textures(std::move(orig.textures)),
        renderBuffers(std::move(orig.renderBuffers)),
        width(orig.width),
        height(orig.height)
    {
        orig.isBackbuffer = false;
        orig.desc = FrameBufferDescriptor();
        orig.width = 0;
        orig.height = 0;
    }

    /**
     *  Assigns a copy of another frame buffer.
     *  @param orig the original frame buffer.
     */
    FrameBuffer& FrameBuffer::operator=(const FrameBuffer& orig)
    {
        if (this != &orig) {
            FrameBuffer tmp{ orig };
            std::swap(*this, tmp);
        }
        return *this;
    }

    /**
     *  Assigns another frame buffer by moving its contents.
     *  @param orig the original frame buffer.
     */
    FrameBuffer& FrameBuffer::operator=(FrameBuffer&& orig)
    {
        if (this != &orig) {
            this->~FrameBuffer();
            fbo = std::move(orig.fbo);
            isBackbuffer = orig.isBackbuffer;
            orig.isBackbuffer = false;
            desc = orig.desc;
            orig.desc = FrameBufferDescriptor();
            textures = std::move(orig.textures);
            renderBuffers = std::move(orig.renderBuffers);
            width = orig.width;
            orig.width = 0;
            height = orig.height;
            orig.height = 0;
        }
        return *this;
    }

    /**
     *  Destructor.
     */
    FrameBuffer::~FrameBuffer()
    {
        textures.clear();
        renderBuffers.clear();
    }

    /**
     * Resizes the frame buffer and re-initializes it if needed.
     * @param fbWidth the new width
     * @param fbHeight the new height
     */
    void FrameBuffer::Resize(unsigned int fbWidth, unsigned int fbHeight)
    {
        if (width == fbWidth && height == fbHeight) return;
        width = fbWidth;
        height = fbHeight;

        if (isBackbuffer) return;

        fbo = std::move(FramebufferRAII());
        OGL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, fbo);
        unsigned int colorAtt = 0;
        drawBuffers.clear();
        textures.clear();
        for (const auto& texDesc : desc.texDesc_) {
            TextureRAII tex;
            OGL_CALL(glBindTexture, texDesc.texType_, tex);
            if (texDesc.texType_ == GL_TEXTURE_CUBE_MAP) {
                for (auto i = 0; i < 6; ++i) {
                    OGL_CALL(glTexImage2D, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, texDesc.texDesc_.internalFormat, width, height, 0, texDesc.texDesc_.format, texDesc.texDesc_.type, nullptr);
                }
            } else {
                if (desc.numSamples_ == 1) { OGL_CALL(glTexImage2D, texDesc.texType_, 0, texDesc.texDesc_.internalFormat, width, height, 0, texDesc.texDesc_.format, texDesc.texDesc_.type, nullptr); }
                else { OGL_CALL(glTexImage2DMultisample, texDesc.texType_, desc.numSamples_, texDesc.texDesc_.internalFormat, width, height, GL_TRUE); }
            }
            std::unique_ptr<GLTexture> texture{ new GLTexture{ std::move(tex), texDesc.texType_, texDesc.texDesc_ } };

            if (texDesc.texType_ == GL_TEXTURE_CUBE_MAP) {
                for (auto i = 0; i < 6; ++i) {
                    auto attachment = findAttachment(texDesc.texDesc_.internalFormat, colorAtt, drawBuffers);
                    OGL_CALL(glFramebufferTexture2D, GL_FRAMEBUFFER, attachment, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, texture->GetGLIdentifier().textureId, 0);
                }
            } else {
                auto attachment = findAttachment(texDesc.texDesc_.internalFormat, colorAtt, drawBuffers);
                OGL_CALL(glFramebufferTexture, GL_FRAMEBUFFER, attachment, texture->GetGLIdentifier().textureId, 0);
            }
            textures.emplace_back(std::move(texture));
        }

        renderBuffers.clear();
        for (const auto& rbDesc : desc.rbDesc_) {
            RenderbufferRAII rb;
            OGL_CALL(glBindRenderbuffer, GL_RENDERBUFFER, rb);
            if (desc.numSamples_ == 1) { OGL_CALL(glRenderbufferStorage, GL_RENDERBUFFER, rbDesc.internalFormat_, width, height); }
            else { OGL_CALL(glRenderbufferStorageMultisample, GL_RENDERBUFFER, desc.numSamples_, rbDesc.internalFormat_, width, height); }
            auto attachment = findAttachment(rbDesc.internalFormat_, colorAtt, drawBuffers);
            OGL_CALL(glFramebufferRenderbuffer, GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, rb);
            renderBuffers.emplace_back(std::move(rb));
        }

        OGL_CALL(glDrawBuffers, static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());

        auto fboStatus = OGL_CALL(glCheckFramebufferStatus, GL_FRAMEBUFFER);
        if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
            throw std::runtime_error("Could not create frame buffer.");
    }

    void FrameBuffer::ResolveFramebufferColor(FrameBuffer* fb, unsigned int readBufferIndex, unsigned int drawBufferIndex) const
    {
        OGL_CALL(glBindFramebuffer, GL_READ_FRAMEBUFFER, fbo);
        OGL_CALL(glReadBuffer, drawBuffers[readBufferIndex]);
        OGL_CALL(glBindFramebuffer, GL_DRAW_FRAMEBUFFER, fb->fbo);
        OGL_CALL(glDrawBuffer, fb->drawBuffers[drawBufferIndex]);
        OGL_CALL(glBlitFramebuffer, 0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        OGL_CALL(glBindFramebuffer, GL_READ_FRAMEBUFFER, 0);
        OGL_CALL(glBindFramebuffer, GL_DRAW_FRAMEBUFFER, 0);
    }

    void FrameBuffer::ResolveFramebufferDepthStencil(FrameBuffer* fb, bool depth, bool stencil) const
    {
        OGL_CALL(glBindFramebuffer, GL_READ_FRAMEBUFFER, fbo);
        OGL_CALL(glBindFramebuffer, GL_DRAW_FRAMEBUFFER, fb->fbo);
        GLbitfield mask = 0;
        if (depth) mask |= GL_DEPTH_BUFFER_BIT;
        if (stencil) mask |= GL_STENCIL_BUFFER_BIT;
        OGL_CALL(glBlitFramebuffer, 0, 0, width, height, 0, 0, width, height, mask, GL_NEAREST);
        OGL_CALL(glBindFramebuffer, GL_READ_FRAMEBUFFER, 0);
        OGL_CALL(glBindFramebuffer, GL_DRAW_FRAMEBUFFER, 0);
    }

    /**
     * Use this frame buffer object as target for rendering.
     */
    void FrameBuffer::UseAsRenderTarget()
    {
        OGL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, fbo);
        if (!isBackbuffer) OGL_CALL(glDrawBuffers, static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
        OGL_CALL(glViewport, 0, 0, width, height);
        OGL_CALL(glScissor, 0, 0, width, height);
    }

    /**
    * Use this frame buffer object as target for rendering and select the draw buffers used.
    * @param drawBufferIndices the indices in the draw buffer to be used.
    */
    void FrameBuffer::UseAsRenderTarget(const std::vector<unsigned int> drawBufferIndices)
    {
        assert(!isBackbuffer);
        std::vector<GLenum> drawBuffersReduced(drawBuffers.size());
        for (unsigned int i = 0; i < drawBufferIndices.size(); ++i) drawBuffersReduced[i] = drawBuffers[drawBufferIndices[i]];

        OGL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, fbo);
        OGL_CALL(glDrawBuffers, static_cast<GLsizei>(drawBuffersReduced.size()), drawBuffersReduced.data());
        OGL_CALL(glViewport, 0, 0, width, height);
        OGL_CALL(glScissor, 0, 0, width, height);
    }

    unsigned int FrameBuffer::findAttachment(GLenum internalFormat, unsigned int& colorAtt, std::vector<GLenum> &drawBuffers)
    {
        GLenum attachment;
        switch (internalFormat)
        {
        case GL_DEPTH_STENCIL:
        case GL_DEPTH24_STENCIL8:
        case GL_DEPTH32F_STENCIL8:
            attachment = GL_DEPTH_STENCIL_ATTACHMENT;
            break;
        case GL_DEPTH_COMPONENT:
        case GL_DEPTH_COMPONENT16:
        case GL_DEPTH_COMPONENT32:
        case GL_DEPTH_COMPONENT24:
        case GL_DEPTH_COMPONENT32F:
            attachment = GL_DEPTH_ATTACHMENT;
            break;
        case GL_STENCIL_INDEX:
        case GL_STENCIL_INDEX1:
        case GL_STENCIL_INDEX4:
        case GL_STENCIL_INDEX8:
        case GL_STENCIL_INDEX16:
            attachment = GL_STENCIL_ATTACHMENT;
            break;
        default:
            attachment = GL_COLOR_ATTACHMENT0 + colorAtt++;
            drawBuffers.emplace_back(attachment);
            break;
        }
        return attachment;
    }

}
