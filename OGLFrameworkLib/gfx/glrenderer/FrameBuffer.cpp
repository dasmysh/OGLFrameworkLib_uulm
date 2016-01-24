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
        desc(d)
    {
        Resize(fbWidth, fbHeight);
    }

    /**
     *  Copy constructor for a frame buffer.
     *  @param orig the original frame buffer.
     */
    FrameBuffer::FrameBuffer(const FrameBuffer& orig) :
        isBackbuffer(false),
        desc(orig.desc)
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
        width = fbWidth;
        height = fbHeight;

        if (isBackbuffer) return;

        fbo = std::move(FramebufferRAII());
        OGL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, fbo.get());
        unsigned int colorAtt = 0;
        drawBuffers.clear();
        for (const auto& texDesc : desc.texDesc) {
            TextureRAII tex;
            OGL_CALL(glBindTexture, GL_TEXTURE_2D, tex.get());
            OGL_CALL(glTexImage2D, GL_TEXTURE_2D, 0, texDesc.internalFormat, width, height, 0, texDesc.format, texDesc.type, nullptr);
            std::unique_ptr<GLTexture> texture{ new GLTexture{ std::move(tex), GL_TEXTURE_2D, texDesc } };

            auto attachment = findAttachment(texDesc.internalFormat, colorAtt, drawBuffers);
            OGL_CALL(glFramebufferTexture, GL_FRAMEBUFFER, attachment, tex.get(), 0);
            textures.emplace_back(std::move(texture));
        }


        for (const auto& rbDesc : desc.rbDesc) {
            RenderbufferRAII rb;
            OGL_CALL(glBindRenderbuffer, GL_RENDERBUFFER, rb.get());
            OGL_CALL(glRenderbufferStorage, GL_RENDERBUFFER, rbDesc.internalFormat, width, height);
            auto attachment = findAttachment(rbDesc.internalFormat, colorAtt, drawBuffers);
            OGL_CALL(glFramebufferRenderbuffer, GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, rb.get());
            renderBuffers.emplace_back(std::move(rb));
        }

        OGL_CALL(glDrawBuffers, static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());

        auto fboStatus = OGL_CALL(glCheckFramebufferStatus, GL_FRAMEBUFFER);
        if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
            throw std::runtime_error("Could not create frame buffer.");
    }

    /**
     * Use this frame buffer object as target for rendering.
     */
    void FrameBuffer::UseAsRenderTarget()
    {
        OGL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, fbo.get());
        if (!isBackbuffer) OGL_CALL(glDrawBuffers, static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
        OGL_CALL(glViewport, 0, 0, width, height);
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

        OGL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, fbo.get());
        OGL_CALL(glDrawBuffers, static_cast<GLsizei>(drawBuffersReduced.size()), drawBuffersReduced.data());
        OGL_CALL(glViewport, 0, 0, width, height);
    }

    unsigned int FrameBuffer::findAttachment(GLenum internalFormat, unsigned int& colorAtt, std::vector<GLenum> &drawBuffers)
    {
        GLenum attachment;
        switch (internalFormat)
        {
        case GL_DEPTH_STENCIL:
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
