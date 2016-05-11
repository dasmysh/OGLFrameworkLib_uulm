/**
 * @file   GLUniformBuffer.h
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2014.01.25
 *
 * @brief  Contains the definition of GLUniformBuffer.
 */

#ifndef GLUNIFORMBUFFER_H
#define GLUNIFORMBUFFER_H

#include "main.h"
#include "core/owned_ptr.h"

namespace cgu {

    class GLBuffer;
    class ShaderBufferBindingPoints;

    /**
     * @brief  Represents uniform buffers.
     *
     * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
     * @date   2014.01.25
     */
    class GLUniformBuffer final
    {
    public:
        GLUniformBuffer(const std::string& name, unsigned int size, ShaderBufferBindingPoints* bindings);
        GLUniformBuffer(const GLUniformBuffer&);
        GLUniformBuffer& operator=(const GLUniformBuffer&);
        GLUniformBuffer(GLUniformBuffer&&);
        GLUniformBuffer& operator=(GLUniformBuffer&&);
        ~GLUniformBuffer();

        GLBuffer* GetBuffer() { return buffer_; }
        const GLBuffer* GetBuffer() const { return buffer_; }
        void UploadData(unsigned int offset, unsigned int size, const void* data);
        void BindBuffer() const;
        ShaderBufferBindingPoints* GetBindingPoints() const { return bindingPoints_; }
        const std::string& GetUBOName() const { return uboName_; }

    private:
        GLUniformBuffer(const std::string& name, ShaderBufferBindingPoints* bindings);

        /** holds the buffer object. */
        owned_ptr<GLBuffer> buffer_;
        /** holds the uniform buffer binding points. */
        ShaderBufferBindingPoints* bindingPoints_;
        /** holds the buffer binding point. */
        GLuint bindingPoint_;
        /** Holds the uniform buffers name. */
        std::string uboName_;

    };
}

#endif /* GLUNIFORMBUFFER_H */
