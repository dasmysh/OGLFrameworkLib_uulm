/**
 * @file   ShaderBufferObject.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.06.15
 *
 * @brief  Declaration of an open gl shader buffer object.
 */

#ifndef SHADERBUFFEROBJECT_H
#define SHADERBUFFEROBJECT_H

#include "main.h"
#include <core/owned_ptr.h>

namespace cgu {
    
    class GLBuffer;
    class ShaderBufferBindingPoints;

    /**
     * @brief Wrapper to access OpenGL shader storage buffer objects (SSBOs).
     */
    class ShaderBufferObject
    {
    public:
        ShaderBufferObject(const std::string& name, GLBuffer* buffer, ShaderBufferBindingPoints* bindings, bool cpuAccess = false);
        ShaderBufferObject(const std::string& name, ShaderBufferBindingPoints* bindings, bool cpuAccess = false);
        // ShaderBufferObject(const std::string& name, unsigned int size, ShaderBufferBindingPoints* bindings, bool cpuAccess = false);
        ShaderBufferObject(const ShaderBufferObject&);
        ShaderBufferObject& operator=(const ShaderBufferObject&);
        ShaderBufferObject(ShaderBufferObject&&);
        ShaderBufferObject& operator=(ShaderBufferObject&&);
        ~ShaderBufferObject();

        GLBuffer* GetBuffer() { return buffer_; }
        const GLBuffer* GetBuffer() const { return buffer_; }
        void BindBuffer() const;
        // void UploadData(unsigned int offset, unsigned int size, const void* data) const;
        // void DownloadData(unsigned int size, void* data) const;

    private:
        ShaderBufferObject(GLuint bindingPoint, ShaderBufferBindingPoints* bindings);

        /** holds the buffer object. */
        owned_ptr<GLBuffer> buffer_;
        /** holds the shader buffer objects binding points. */
        ShaderBufferBindingPoints* bindingPoints_;
        /** holds the buffer binding point. */
        GLuint bindingPoint_;
    };
}

#endif /* SHADERBUFFEROBJECT_H */
