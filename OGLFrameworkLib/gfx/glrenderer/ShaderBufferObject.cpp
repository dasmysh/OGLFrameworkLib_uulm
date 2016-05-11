/**
 * @file   ShaderBufferObject.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.06.15
 *
 * @brief  Implementation of shader buffer objects.
 */

#include "ShaderBufferObject.h"
#include "ShaderBufferBindingPoints.h"
#include "GLBuffer.h"

namespace cgu {

    ShaderBufferObject::ShaderBufferObject(const std::string& name, GLBuffer* buffer, ShaderBufferBindingPoints* bindings, bool cpuAccess) :
        buffer_{ buffer, false },
        bindingPoints_(bindings),
        bindingPoint_(bindingPoints_->GetBindingPoint(name))
    {
    }

    /**
         * Constructor.
         * @param name the name of the shader buffer object.
         * @param bindings the binding points used to bind the buffer to
         */
    ShaderBufferObject::ShaderBufferObject(const std::string& name, ShaderBufferBindingPoints* bindings, bool cpuAccess) :
        buffer_{ make_owned<GLBuffer>(cpuAccess ? GL_STREAM_READ : GL_DYNAMIC_DRAW) },
        bindingPoints_(bindings),
        bindingPoint_(bindingPoints_->GetBindingPoint(name))
    {
        OGL_CALL(glBindBuffer, GL_SHADER_STORAGE_BUFFER, buffer_->GetBuffer());
        OGL_CALL(glBindBuffer, GL_SHADER_STORAGE_BUFFER, 0);
    }

    /**
     * Constructor.
     * @param name the name of the shader buffer object.
     * @param size the size of the buffer it is to be created with
     * @param bindings the binding points used to bind the buffer to
     */
    /*ShaderBufferObject::ShaderBufferObject(const std::string& name, unsigned int size, ShaderBufferBindingPoints* bindings, bool cpuAccess) :
        ShaderBufferObject(name, bindings, cpuAccess)
    {
        OGL_CALL(glBindBuffer, GL_SHADER_STORAGE_BUFFER, buffer_->GetBuffer());
        buffer_->InitializeData(size, nullptr);
        OGL_CALL(glBindBuffer, GL_SHADER_STORAGE_BUFFER, 0);
        BindBuffer();
    }*/

    ShaderBufferObject::ShaderBufferObject(GLuint bindingPoint, ShaderBufferBindingPoints* bindings) :
        bindingPoints_(bindings),
        bindingPoint_(bindingPoint)
    {
    }

    ShaderBufferObject::ShaderBufferObject(const ShaderBufferObject& rhs) :
        ShaderBufferObject(rhs.bindingPoint_, rhs.bindingPoints_)
    {
        if (buffer_.is_owned()) buffer_ = make_owned<GLBuffer>(*rhs.buffer_);
        else buffer_ = owned_ptr<GLBuffer>(rhs.buffer_, false);
    }

    ShaderBufferObject& ShaderBufferObject::operator=(const ShaderBufferObject& rhs)
    {
        if (this != &rhs) {
            ShaderBufferObject tmp{ rhs };
            std::swap(*this, tmp);
        }
        return *this;
    }

    ShaderBufferObject::ShaderBufferObject(ShaderBufferObject&& rhs) :
        buffer_(std::move(rhs.buffer_)),
        bindingPoints_(std::move(rhs.bindingPoints_)),
        bindingPoint_(std::move(rhs.bindingPoint_))
    {
    }

    ShaderBufferObject& ShaderBufferObject::operator=(ShaderBufferObject&& rhs)
    {
        this->~ShaderBufferObject();
        buffer_ = std::move(rhs.buffer_);
        bindingPoints_ = std::move(rhs.bindingPoints_);
        bindingPoint_ = std::move(rhs.bindingPoint_);
        return *this;
    }

    ShaderBufferObject::~ShaderBufferObject() = default;

    void ShaderBufferObject::BindBuffer() const
    {
        OGL_CALL(glBindBufferBase, GL_SHADER_STORAGE_BUFFER, bindingPoint_, buffer_->GetBuffer());
    }

    /*void ShaderBufferObject::UploadData(unsigned int offset, unsigned int size, const void* data) const
    {
        OGL_CALL(glBindBuffer, GL_SHADER_STORAGE_BUFFER, ssbo);
        if (offset + size > bufferSize) {
            std::vector<int8_t> tmp(offset);
            OGL_CALL(glGetBufferSubData, GL_SHADER_STORAGE_BUFFER, 0, offset, tmp.data());
            OGL_CALL(glBufferData, GL_SHADER_STORAGE_BUFFER, offset + size, nullptr, GL_DYNAMIC_DRAW);
            OGL_CALL(glBufferSubData, GL_SHADER_STORAGE_BUFFER, 0, offset, tmp.data());
        }

        OGL_CALL(glBufferSubData, GL_SHADER_STORAGE_BUFFER, offset, size, data);
        OGL_CALL(glBindBuffer, GL_SHADER_STORAGE_BUFFER, 0);
    }*/

    /*void ShaderBufferObject::DownloadData(unsigned size, void* data) const
    {
        OGL_CALL(glBindBuffer, GL_SHADER_STORAGE_BUFFER, ssbo);
        auto p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
        memcpy(data, p, size);
        OGL_CALL(glUnmapBuffer, GL_SHADER_STORAGE_BUFFER);
    }*/
}
