/**
 * @file   Shader.h
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2014.01.15
 *
 * @brief  Contains the definition of Shader.
 */

#ifndef SHADER_H
#define SHADER_H

#include "main.h"

namespace cgu {

    class GPUProgram;

    using compiler_error_info = boost::error_info<struct tag_compiler_error, std::string>;
    using fileid_info = boost::error_info<struct tag_fileid, unsigned int>;
    using lineno_info = boost::error_info<struct tag_lineno, unsigned int>;

    /**
     * Exception class for shader compiler errors.
     */
    struct shader_compiler_error : virtual resource_loading_error { };

    /**
     * @brief  The resource type for shaders.
     *
     * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
     * @date   2014.01.15
     */
    class Shader : public Resource
    {
    public:
        Shader(const std::string& shaderFilename, ApplicationBase* app);
        Shader(const Shader&);
        Shader& operator=(const Shader&);
        Shader(Shader&&);
        Shader& operator=(Shader&&);
        virtual ~Shader();

        void ResetShader(GLuint newShader);
        ShaderRAII RecompileShader() const;

    private:
        friend GPUProgram;

        /** Holds the compiled shader. */
        ShaderRAII shader;
        /** Holds the shaders type. */
        GLenum type;
        /** Holds the shaders type as a string. */
        std::string strType;

        ShaderRAII CompileShader(const std::string& filename, const std::vector<std::string>& defines, GLenum type, const std::string& strType) const;
        std::string LoadShaderFile(const std::string& filename, const std::vector<std::string>& defines, unsigned int& fileId, unsigned int recursionDepth) const;
    };
}

#endif /* SHADER_H */
