/**
 * @file   GPUProgram.h
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2014.01.15
 *
 * @brief  Contains the definition of GPUProgram.
 */

#ifndef GPUPROGRAM_H
#define GPUPROGRAM_H

#include "main.h"
#include "GLVertexAttributeArray.h"

namespace cgu {

    class ShaderBufferBindingPoints;
    class ApplicationBase;
    class Shader;

    /**
     * @brief  Complete GPU program with multiple Shader objects working together.
     *
     * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
     * @date   2014.01.15
     */
    class GPUProgram : public Resource
    {
    public:
        GPUProgram(const std::string programName, ApplicationBase* app);
        GPUProgram(const GPUProgram&);
        GPUProgram& operator=(const GPUProgram&);
        GPUProgram(GPUProgram&&);
        GPUProgram& operator=(GPUProgram&&);
        virtual ~GPUProgram();

        void RecompileProgram();

        GLVertexAttributeArray* CreateVertexAttributeArray(GLuint vBuffer, GLuint iBuffer);
        std::vector<BindingLocation> GetAttributeLocations(const std::vector<std::string>& attribNames);
        BindingLocation GetUniformLocation(const std::string& uniformName);
        std::vector<BindingLocation> GetUniformLocations(const std::vector<std::string>& uniformNames);
        void SetUniform(BindingLocation name, float data) const;
        void SetUniform(BindingLocation name, const glm::vec2& data) const;
        void SetUniform(BindingLocation name, const glm::vec3& data) const;
        void SetUniform(BindingLocation name, const glm::vec3* data, unsigned int elements) const;
        void SetUniform(BindingLocation name, const glm::mat3& data) const;
        void SetUniform(BindingLocation name, const glm::vec4& data) const;
        void SetUniform(BindingLocation name, const glm::vec4* data, unsigned int elements) const;
        void SetUniform(BindingLocation name, const glm::mat4& data) const;
        void SetUniform(BindingLocation name, const std::vector<float>& data) const;
        void SetUniform(BindingLocation name, const float* data, unsigned int elements) const;
        void SetUniform(BindingLocation name, int data) const;
        void SetUniform(BindingLocation name, const glm::ivec2& data) const;
        void SetUniform(BindingLocation name, const std::vector<int>& data) const;
        void SetUniform(BindingLocation name, unsigned int data) const;
        void SetUniform(BindingLocation name, const glm::uvec3& data) const;
        BindingLocation GetUniformBufferLocation(const std::string& uBufferName);
        void BindUniformBlock(const std::string& uBufferName, ShaderBufferBindingPoints& bindingPoints);
        void BindUniformBlock(const std::string& uBufferName, GLuint bindingPoint);
        void BindUniformBlock(BindingLocation name, GLuint bindingPoint) const;
        BindingLocation GetShaderBufferLocation(const std::string& sBufferName);
        void BindShaderBuffer(const std::string& sBufferName, ShaderBufferBindingPoints& bindingPoints);
        void BindShaderBuffer(const std::string& sBufferName, GLuint bindingPoint);
        void BindShaderBuffer(BindingLocation name, GLuint bindingPoint) const;

        void UseProgram() const;

    private:
        typedef std::unique_ptr<shader_binding_desc> BindingLocationInternal;
        /** Holds the program. */
        ProgramRAII program;
        /** Holds all shaders in the program. */
        std::vector<std::shared_ptr<Shader>> shaders;
        /** holds the known vertex attribute bindings. */
        std::unordered_map<std::string, BindingLocationInternal> knownVABindings;
        /** holds the known uniform locations. */
        std::unordered_map<std::string, BindingLocationInternal> knownUniformBindings;
        /** holds the known uniform buffer locations. */
        std::unordered_map<std::string, BindingLocationInternal> knownUBBindings;
        /** holds the bound uniform blocks. */
        std::unordered_map<std::string, GLuint> boundUBlocks;
        /** holds the known SSBO locations. */
        std::unordered_map<std::string, BindingLocationInternal> knownSSBOBindings;
        /** holds the bound SSBO. */
        std::unordered_map<std::string, GLuint> boundSSBOs;
        /** holds the vertex attribute arrays associated with this GPU program. */
        std::vector<std::unique_ptr<GLVertexAttributeArray> > vaos;

        void LoadInternal(GLuint newProgram);
        GLuint LinkNewProgram(const std::vector<GLuint>& shaders) const;
    };
}

#endif /* GPUPROGRAM_H */
