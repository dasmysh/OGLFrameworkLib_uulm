/**
 * @file   GPUProgram.cpp
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2014.01.15
 *
 * @brief  Contains the implementation of GPUProgram.
 */

#include "GPUProgram.h"
#include "ShaderBufferBindingPoints.h"
#include "app/ApplicationBase.h"
#include <glm/gtc/type_ptr.hpp>

namespace cgu {

    /**
     * Constructor.
     * @param programName the programs name
     * @param win the base window this manager belongs to holding all needed dependencies.
     */
    GPUProgram::GPUProgram(const std::string programName, ApplicationBase* app) :
        Resource(programName, app),
        knownVABindings(),
        knownUniformBindings(),
        knownUBBindings(),
        boundUBlocks(),
        knownSSBOBindings(),
        boundSSBOs(),
        vaos()
    {
        auto programNames = GetSubresourceIds();
        std::vector<GLuint> shaderObjs;
        for (auto& progName : programNames) {
            // ignore exception and reload whole program
            auto shader = std::move(application->GetShaderManager()->GetResource(progName));
            shaderObjs.emplace_back(static_cast<GLuint>(shader->shader));
            shaders.emplace_back(std::move(shader));
        }
        LoadInternal(LinkNewProgram(shaderObjs));
    }

    /** Destructor. */
    GPUProgram::~GPUProgram() = default;

    /** Copy constructor. */
    GPUProgram::GPUProgram(const GPUProgram& rhs) : GPUProgram(rhs.getId(), rhs.application)
    {
    }

    /** Copy assignment operator. */
    GPUProgram& GPUProgram::operator=(const GPUProgram& rhs)
    {
        GPUProgram tmp{ rhs };
        std::swap(*this, tmp);
        return *this;
    }

    /**
     * Move constructor.
     * @param rhs the original object
     */
    GPUProgram::GPUProgram(GPUProgram&& rhs) :
        Resource(std::move(rhs)),
        program(std::move(rhs.program)),
        knownVABindings(std::move(rhs.knownVABindings)),
        knownUniformBindings(std::move(rhs.knownUniformBindings)),
        knownUBBindings(std::move(rhs.knownUBBindings)),
        boundUBlocks(std::move(rhs.boundUBlocks)),
        knownSSBOBindings(std::move(rhs.knownSSBOBindings)),
        boundSSBOs(std::move(rhs.boundSSBOs)),
        vaos(std::move(rhs.vaos))
    {
    }

    /**
     * Move assignment operator.
     * @param rhs the original object
     * @return reference to this object
     */
    GPUProgram& GPUProgram::operator =(GPUProgram&& rhs)
    {
        if (this != &rhs) {
            this->~GPUProgram();
            Resource* tRes = this;
            *tRes = static_cast<Resource&&> (std::move(rhs));
            program = std::move(rhs.program);
            knownVABindings = std::move(rhs.knownVABindings);
            knownUniformBindings = std::move(rhs.knownUniformBindings);
            knownUBBindings = std::move(rhs.knownUBBindings);
            boundUBlocks = std::move(rhs.boundUBlocks);
            knownSSBOBindings = std::move(rhs.knownSSBOBindings);
            boundSSBOs = std::move(rhs.boundSSBOs);
            vaos = std::move(rhs.vaos);
        }
        return *this;
    }

    /**
     * Internal load function to be called after the program has been initialized.
     * @param newProgram the new initialized program to set
     */
    void GPUProgram::LoadInternal(GLuint newProgram)
    {
        program.reset(newProgram);

        for (auto& ab : knownVABindings) {
            ab.second->iBinding = OGL_CALL(glGetAttribLocation, this->program, ab.first.c_str());
        }

        for (auto& ub : knownUniformBindings) {
            ub.second->iBinding = OGL_CALL(glGetUniformLocation, this->program, ub.first.c_str());
        }

        for (auto& ubb : knownUBBindings) {
            ubb.second->uBinding = OGL_CALL(glGetUniformBlockIndex, this->program, ubb.first.c_str());
        }

        for (auto& bublock : boundUBlocks) {
            BindUniformBlock(bublock.first, bublock.second);
        }

        for (auto& ssbo : knownSSBOBindings) {
            ssbo.second->uBinding = OGL_CALL(glGetProgramResourceIndex, this->program, GL_SHADER_STORAGE_BLOCK, ssbo.first.c_str());
        }

        for (auto& ssbo : boundSSBOs) {
            BindShaderBuffer(ssbo.first, ssbo.second);
        }

        for (auto& vao : vaos) {
            vao->DisableAttributes();
            vao->UpdateVertexAttributes();
        }
    }

    /** Recompiles the program. */
    void GPUProgram::RecompileProgram()
    {
        auto shaderIds = GetSubresourceIds();
        std::vector<GLuint> newOGLShaders;
        for (auto& shaderId : shaderIds) {
            shaders.emplace_back(std::move(application->GetShaderManager()->GetResource(shaderId)));
        }
        for (unsigned int i = 0; i < shaders.size(); ++i) {
            newOGLShaders.emplace_back(ShaderRAII(0));
            while (!newOGLShaders[i]) {
                try {
                    newOGLShaders[i] = std::move(shaders[i]->RecompileShader());
                }
                catch (shader_compiler_error compilerError) {
                    throw;
                }
            }
        }

        GLuint tempProgram = 0;
        try {
            tempProgram = LinkNewProgram(newOGLShaders);
        }
        catch (shader_compiler_error compilerError) {
            throw;
        }

        program.reset();
        for (unsigned int i = 0; i < shaders.size(); ++i) {
            shaders[i]->ResetShader(std::move(ShaderRAII(newOGLShaders[i])));
        }
        LoadInternal(tempProgram);
    }

    /**
     * Get the vertex attribute locations in this program.
     * @param attribNames the names of the attributes to find
     * @return the list of locations
     */
    std::vector<BindingLocation> GPUProgram::GetAttributeLocations(const std::vector<std::string>& attribNames)
    {
        std::vector<BindingLocation> result;
        result.reserve(attribNames.size());
        for (const auto& name : attribNames) {
            try {
                result.push_back(knownVABindings.at(name).get());
            }
            catch (std::out_of_range e) {
                BindingLocationInternal binding(new shader_binding_desc());
                binding->iBinding = OGL_CALL(glGetAttribLocation, this->program, name.c_str());
                result.push_back(knownVABindings.insert(std::make_pair(name, std::move(binding))).first->second.get());
            }
        }
        return result;
    }

    /**
     * Creates a new vertex attribute array.
     * @param vBuffer the vertex buffer the array is for
     * @param iBuffer the index buffer used
     * @return pointer to the new vertex attribute array
     */
    GLVertexAttributeArray* GPUProgram::CreateVertexAttributeArray(GLuint vBuffer, GLuint iBuffer)
    {
        vaos.push_back(std::make_unique<GLVertexAttributeArray>(vBuffer, iBuffer));
        return vaos.back().get();
    }

    /**
     * Get an uniform location of this program.
     * @param uniformName the name of the uniform
     * @return the uniforms location
     */
    BindingLocation GPUProgram::GetUniformLocation(const std::string& uniformName)
    {
        try {
            return knownUniformBindings.at(uniformName).get();
        }
        catch (std::out_of_range e) {
            BindingLocationInternal binding(new shader_binding_desc());
            binding->iBinding = OGL_CALL(glGetUniformLocation, this->program, uniformName.c_str());
            return knownUniformBindings.insert(std::make_pair(uniformName, std::move(binding))).first->second.get();
        }
    }

    /**
     * Get locations of the uniforms by their names.
     * @param uniformNames the uniform names
     * @return a vector of the locations
     */
    std::vector<BindingLocation> GPUProgram::GetUniformLocations(const std::vector<std::string>& uniformNames)
    {
        std::vector<BindingLocation> result;
        result.reserve(uniformNames.size());
        for (const auto& name : uniformNames) {
            try {
                result.push_back(knownUniformBindings.at(name).get());
            }
            catch (std::out_of_range e) {
                BindingLocationInternal binding(new shader_binding_desc());
                binding->iBinding = OGL_CALL(glGetUniformLocation, this->program, name.c_str());
                result.push_back(knownUniformBindings.insert(std::make_pair(name, std::move(binding))).first->second.get());
            }
        }
        return result;
    }

    /**
     * Returns the OpenGL location of the UBO with given name.
     * @param uBufferName the uniform buffer name
     * @return the UBO location
     */
    BindingLocation GPUProgram::GetUniformBufferLocation(const std::string& uBufferName)
    {
        try {
            return knownUBBindings.at(uBufferName).get();
        }
        catch (std::out_of_range e) {
            BindingLocationInternal binding(new shader_binding_desc());
            binding->uBinding = OGL_CALL(glGetUniformBlockIndex, this->program, uBufferName.c_str());
            if (binding->uBinding == GL_INVALID_INDEX)
                throw std::out_of_range("Could not find uniform buffer \"" + uBufferName + "\".");
            return knownUBBindings.insert(std::make_pair(uBufferName, std::move(binding))).first->second.get();
        }
    }

    /**
     * Sets a uniform with given OpenGL name/location (vec2 version)
     * @param name the location of the uniform
     * @param data the vec2 to set the uniform to
     */
    void GPUProgram::SetUniform(BindingLocation name, const glm::vec2& data) const
    {
        GLuint cProg;
        OGL_CALL(glGetIntegerv, GL_CURRENT_PROGRAM, reinterpret_cast<GLint*>(&cProg));
        assert(program == cProg);
        OGL_CALL(glUniform2fv, name->iBinding, 1, reinterpret_cast<const GLfloat*> (&data));
    }

    /**
     * Sets a uniform with given OpenGL name/location (vec3 version)
     * @param name the location of the uniform
     * @param data the vec3 to set the uniform to
     */
    void GPUProgram::SetUniform(BindingLocation name, const glm::vec3& data) const
    {
        GLuint cProg;
        OGL_CALL(glGetIntegerv, GL_CURRENT_PROGRAM, reinterpret_cast<GLint*>(&cProg));
        assert(program == cProg);
        OGL_CALL(glUniform3fv, name->iBinding, 1, reinterpret_cast<const GLfloat*> (&data));
    }

    /**
     * Sets a uniform with given OpenGL name/location (mat4 version)
     * @param name the location of the uniform
     * @param data the mat4 to set the uniform to
     */
    void GPUProgram::SetUniform(BindingLocation name, const glm::mat3& data) const
    {
        GLuint cProg;
        OGL_CALL(glGetIntegerv, GL_CURRENT_PROGRAM, reinterpret_cast<GLint*>(&cProg));
        assert(program == cProg);
        OGL_CALL(glUniformMatrix3fv, name->iBinding, 1, GL_FALSE, glm::value_ptr(data));
    }

    /**
     * Sets a uniform with given OpenGL name/location (vec4 version)
     * @param name the location of the uniform
     * @param data the vec4 to set the uniform to
     */
    void GPUProgram::SetUniform(BindingLocation name, const glm::vec4& data) const
    {
        GLuint cProg;
        OGL_CALL(glGetIntegerv, GL_CURRENT_PROGRAM, reinterpret_cast<GLint*>(&cProg));
        assert(program == cProg);
        OGL_CALL(glUniform4fv, name->iBinding, 1, reinterpret_cast<const GLfloat*> (&data));
    }

    /**
     * Sets a uniform with given OpenGL name/location (mat4 version)
     * @param name the location of the uniform
     * @param data the mat4 to set the uniform to
     */
    void GPUProgram::SetUniform(BindingLocation name, const glm::mat4& data) const
    {
        GLuint cProg;
        OGL_CALL(glGetIntegerv, GL_CURRENT_PROGRAM, reinterpret_cast<GLint*>(&cProg));
        assert(program == cProg);
        OGL_CALL(glUniformMatrix4fv, name->iBinding, 1, GL_FALSE, glm::value_ptr(data));
    }

    /**
     * Sets a uniform with given OpenGL name/location (float[] version)
     * @param name the location of the uniform
     * @param data the float[] to set the uniform to
     */
    void GPUProgram::SetUniform(BindingLocation name, const std::vector<float>& data) const
    {
        GLuint cProg;
        OGL_CALL(glGetIntegerv, GL_CURRENT_PROGRAM, reinterpret_cast<GLint*>(&cProg));
        assert(program == cProg);
        OGL_CALL(glUniform1fv, name->iBinding, static_cast<GLsizei>(data.size()), data.data());
    }

    /**
     * Sets a uniform with given OpenGL name/location (int[] version)
     * @param name the location of the uniform
     * @param data the int[] to set the uniform to
     */
    void GPUProgram::SetUniform(BindingLocation name, const std::vector<int>& data) const
    {
        GLuint cProg;
        OGL_CALL(glGetIntegerv, GL_CURRENT_PROGRAM, reinterpret_cast<GLint*>(&cProg));
        assert(program == cProg);
        OGL_CALL(glUniform1iv, name->iBinding, static_cast<GLsizei>(data.size()), data.data());
    }

    /**
     * Sets a uniform with given OpenGL name/location (int version)
     * @param name the location of the uniform
     * @param data the int to set the uniform to
     */
    void GPUProgram::SetUniform(BindingLocation name, int data) const
    {
        GLuint cProg;
        OGL_CALL(glGetIntegerv, GL_CURRENT_PROGRAM, reinterpret_cast<GLint*>(&cProg));
        assert(program == cProg);
        OGL_CALL(glUniform1i, name->iBinding, data);
    }

    /**
     * Sets a uniform with given OpenGL name/location (ivec2 version)
     * @param name the location of the uniform
     * @param data the ivec2 to set the uniform to
     */
    void GPUProgram::SetUniform(BindingLocation name, const glm::ivec2& data) const
    {
        GLuint cProg;
        OGL_CALL(glGetIntegerv, GL_CURRENT_PROGRAM, reinterpret_cast<GLint*>(&cProg));
        assert(program == cProg);
        OGL_CALL(glUniform2iv, name->iBinding, 1, glm::value_ptr(data));
    }

    /**
     * Sets a uniform with given OpenGL name/location (float version)
     * @param name the location of the uniform
     * @param data the float to set the uniform to
     */
    void GPUProgram::SetUniform(BindingLocation name, float data) const
    {
        GLuint cProg;
        OGL_CALL(glGetIntegerv, GL_CURRENT_PROGRAM, reinterpret_cast<GLint*>(&cProg));
        assert(program == cProg);
        OGL_CALL(glUniform1f, name->iBinding, data);
    }

    /**
    * Sets a uniform with given OpenGL name/location (int version)
    * @param name the location of the uniform
    * @param data the uint vector to set the uniform to
    */
    void GPUProgram::SetUniform(BindingLocation name, const glm::uvec3& data) const
    {
        GLuint cProg;
        OGL_CALL(glGetIntegerv, GL_CURRENT_PROGRAM, reinterpret_cast<GLint*>(&cProg));
        assert(program == cProg);
        OGL_CALL(glUniform3ui, name->iBinding, data.x, data.y, data.z);
    }

    /**
     * Bind a uniform block with given name to a UBO binding point
     * @param uBufferName the UBO name
     * @param bindingPoints the binding points
     */
    void GPUProgram::BindUniformBlock(const std::string& uBufferName, ShaderBufferBindingPoints& bindingPoints)
    {
        auto bindingPoint = bindingPoints.GetBindingPoint(uBufferName);
        BindUniformBlock(uBufferName, bindingPoint);
    }

    /**
     * Bind a uniform block with given name to a UBO binding point
     * @param uBufferName the UBO name
     * @param bindingPoint the binding point
     */
    void GPUProgram::BindUniformBlock(const std::string& uBufferName, GLuint bindingPoint)
    {
        auto uBufferLoc = GetUniformBufferLocation(uBufferName);
        BindUniformBlock(uBufferLoc, bindingPoint);
        boundUBlocks[uBufferName] = bindingPoint;
    }

    /**
     * Bind a uniform block with given name to a UBO binding point
     * @param name the UBO name
     * @param bindingPoint the binding point
     */
    void GPUProgram::BindUniformBlock(BindingLocation name, GLuint bindingPoint) const
    {
        OGL_CALL(glUniformBlockBinding, program, name->uBinding, bindingPoint);
    }

    /**
    * Returns the OpenGL location of the SSBO with given name.
    * @param sBufferName the uniform buffer name
    * @return the SSBO location
    */
    BindingLocation GPUProgram::GetShaderBufferLocation(const std::string& sBufferName)
    {
        try {
            return knownSSBOBindings.at(sBufferName).get();
        }
        catch (std::out_of_range e) {
            BindingLocationInternal binding(new shader_binding_desc());
            binding->uBinding = OGL_CALL(glGetProgramResourceIndex, this->program, GL_SHADER_STORAGE_BLOCK, sBufferName.c_str());
            if (binding->uBinding == GL_INVALID_INDEX)
                throw std::out_of_range("Could not find uniform buffer \"" + sBufferName + "\".");
            return knownSSBOBindings.insert(std::make_pair(sBufferName, std::move(binding))).first->second.get();
        }
    }

    /**
    * Bind a SSBO with given name to a SSBO binding point
    * @param sBufferName the SSBO name
    * @param bindingPoints the binding points
    */
    void GPUProgram::BindShaderBuffer(const std::string& sBufferName, ShaderBufferBindingPoints& bindingPoints)
    {
        auto bindingPoint = bindingPoints.GetBindingPoint(sBufferName);
        BindShaderBuffer(sBufferName, bindingPoint);
    }

    /**
    * Bind a SSBO with given name to a SSBO binding point
    * @param sBufferName the SSBO name
    * @param bindingPoint the binding point
    */
    void GPUProgram::BindShaderBuffer(const std::string& sBufferName, GLuint bindingPoint)
    {
        auto sBufferLoc = GetShaderBufferLocation(sBufferName);
        BindShaderBuffer(sBufferLoc, bindingPoint);
        boundSSBOs[sBufferName] = bindingPoint;
    }

    /**
    * Bind a SSBO with given name to a SSBO binding point
    * @param name the SSBO name
    * @param bindingPoint the binding point
    */
    void GPUProgram::BindShaderBuffer(BindingLocation name, GLuint bindingPoint) const
    {
        OGL_CALL(glShaderStorageBlockBinding, program, name->uBinding, bindingPoint);
    }

    /**
     * Activate this program for rendering use.
     */
    void GPUProgram::UseProgram() const
    {
        OGL_CALL(glUseProgram, program);
    }

    GLuint GPUProgram::LinkNewProgram(const std::vector<GLuint>& shdrs) const
    {
        auto program = OGL_SCALL(glCreateProgram);
        if (program == 0) {
            LOG(ERROR) << L"Could not create GPU program!";
            throw resource_loading_error() << resid_info(getId())
                << errdesc_info("Cannot create program.");
        }
        for (const auto& shader : shdrs) {
            OGL_CALL(glAttachShader, program, shader);
        }
        OGL_CALL(glLinkProgram, program);

        GLint status;
        OGL_CALL(glGetProgramiv, program, GL_LINK_STATUS, &status);
        if (status == GL_FALSE) {
            GLint infoLogLength;
            OGL_CALL(glGetProgramiv, program, GL_INFO_LOG_LENGTH, &infoLogLength);

            auto strInfoLog = new GLchar[infoLogLength + 1];
            OGL_CALL(glGetProgramInfoLog, program, infoLogLength, NULL, strInfoLog);
            LOG(ERROR) << L"Linker failure: " << strInfoLog;
            std::string infoLog = strInfoLog;
            delete[] strInfoLog;

            for (const auto& shader : shdrs) {
                OGL_CALL(glDetachShader, program, shader);
            }
            OGL_CALL(glDeleteProgram, program);

            throw shader_compiler_error() << resid_info(getId())
                << compiler_error_info(infoLog)
                << errdesc_info("Program linking failed.");
        }
        for (const auto& shader : shdrs) {
            OGL_CALL(glDetachShader, program, shader);
        }
        return program;
    }
}
