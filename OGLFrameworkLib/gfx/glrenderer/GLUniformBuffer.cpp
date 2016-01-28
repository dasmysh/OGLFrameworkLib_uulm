/**
 * @file   GLUniformBuffer.cpp
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2014.01.25
 *
 * @brief  Contains the implementation of GLUniformBuffer.
 */

#include "GLUniformBuffer.h"
#include "ShaderBufferBindingPoints.h"

namespace cgu {

    /**
     * Constructor.
     * @param name the name of the uniform buffer.
     * @param size the size of the uniform buffer
     * @param bindings the binding points used to bind the buffer to
     */
    GLUniformBuffer::GLUniformBuffer(const std::string& name, unsigned int size,
        ShaderBufferBindingPoints* bindings) :
        bufferSize(size),
        bindingPoints(bindings),
        bindingPoint(bindingPoints->GetBindingPoint(name)),
        uboName(name)
    {
        OGL_CALL(glBindBuffer, GL_UNIFORM_BUFFER, ubo);
        OGL_CALL(glBufferData, GL_UNIFORM_BUFFER, size, nullptr, GL_STREAM_DRAW);
        OGL_CALL(glBindBuffer, GL_UNIFORM_BUFFER, 0);
        BindBuffer();
    }

    /**
     *  Copy constructor.
     */
    GLUniformBuffer::GLUniformBuffer(const GLUniformBuffer& rhs) :
        GLUniformBuffer(rhs.uboName, bufferSize, bindingPoints)
    {
    }

    /**
     *  Copy assignment operator.
     */
    GLUniformBuffer& GLUniformBuffer::operator=(const GLUniformBuffer& rhs)
    {
        if (this != &rhs) {
            GLUniformBuffer tmp{ rhs };
            std::swap(*this, tmp);
        }
        return *this;
    }

    /**
     *  Move constructor.
     */
    GLUniformBuffer::GLUniformBuffer(GLUniformBuffer&& rhs) :
        ubo(std::move(rhs.ubo)),
        bufferSize(std::move(rhs.bufferSize)),
        bindingPoints(std::move(rhs.bindingPoints)),
        bindingPoint(std::move(rhs.bindingPoint)),
        uboName(std::move(uboName))
    {
    }

    /**
     *  Move assignment operator.
     */
    GLUniformBuffer& GLUniformBuffer::operator=(GLUniformBuffer&& rhs)
    {
        if (this != &rhs) {
            this->~GLUniformBuffer();
            ubo = std::move(rhs.ubo);
            bufferSize = std::move(rhs.bufferSize);
            bindingPoints = std::move(rhs.bindingPoints);
            bindingPoint = std::move(rhs.bindingPoint);
            uboName = std::move(rhs.uboName);
        }
        return *this;
    }

    /** Destructor. */
    GLUniformBuffer::~GLUniformBuffer() = default;

    /**
     * Upload data to the uniform buffer.
     * @param offset the offset into the buffer to store the data
     * @param size the size of the data
     * @param data the data to store in the buffer
     */
    void GLUniformBuffer::UploadData(unsigned int offset, unsigned int size, const void* data) const
    {
        assert((offset + size) <= bufferSize);
        OGL_CALL(glBindBuffer, GL_UNIFORM_BUFFER, ubo);
        OGL_CALL(glBufferSubData, GL_UNIFORM_BUFFER, offset, size, data);
        OGL_CALL(glBindBuffer, GL_UNIFORM_BUFFER, 0);
    }

    void GLUniformBuffer::BindBuffer() const
    {
        OGL_CALL(glBindBufferRange, GL_UNIFORM_BUFFER, bindingPoint, ubo, 0, bufferSize);
    }
}
