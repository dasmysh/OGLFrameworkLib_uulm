/**
 * @file   Shader.cpp
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2014.01.15
 *
 * @brief  Contains the implementation of Shader.
 */

#include "Shader.h"
#include "app/ApplicationBase.h"
#include "app/Configuration.h"

#include <fstream>
#include <boost/algorithm/string/predicate.hpp>
#include <codecvt>
#include <boost/algorithm/string/trim.hpp>
#include <boost/regex.hpp>
#include <boost/filesystem.hpp>

namespace cgu {

    /**
     * Constructor.
     * @param shaderFilename the shaders file name
     */
    Shader::Shader(const std::string& shaderFilename, ApplicationBase* app) :
        Resource(shaderFilename, app),
        shader(0),
        type(GL_VERTEX_SHADER),
        strType("vertex")
    {
        auto shaderDefinition = GetParameters();
        if (boost::ends_with(shaderDefinition[0], ".fp")) {
            type = GL_FRAGMENT_SHADER;
            strType = "fragment";
        } else if (boost::ends_with(shaderDefinition[0], ".gp")) {
            type = GL_GEOMETRY_SHADER;
            strType = "geometry";
        } else if (boost::ends_with(shaderDefinition[0], ".tcp")) {
            type = GL_TESS_CONTROL_SHADER;
            strType = "tesselation control";
        } else if (boost::ends_with(shaderDefinition[0], ".tep")) {
            type = GL_TESS_EVALUATION_SHADER;
            strType = "tesselation evaluation";
        } else if (boost::ends_with(shaderDefinition[0], ".cp")) {
            type = GL_COMPUTE_SHADER;
            strType = "compute";
        }
    }

    /** Copy constructor. */
    Shader::Shader(const Shader& rhs) : Shader(rhs.id, rhs.application)
    {
        if (rhs.IsLoaded()) Shader::Load();
    }

    /** Copy assignment operator. */
    Shader& Shader::operator=(const Shader& rhs)
    {
        Resource* tRes = this;
        *tRes = static_cast<const Resource&>(rhs);
        Shader tmp{ rhs };
        std::swap(shader, tmp.shader);
        std::swap(type, tmp.type);
        std::swap(strType, tmp.strType);
        return *this;
    }

    /** Move constructor. */
    Shader::Shader(Shader&& rhs) :
        Resource(std::move(rhs)),
        shader(rhs.shader),
        type(rhs.type),
        strType(std::move(rhs.strType))
    {
        rhs.shader = 0;
        rhs.type = GL_VERTEX_SHADER;
    }

    /** Move assignment operator. */
    Shader& Shader::operator =(Shader&& rhs)
    {
        this->~Shader();
        Resource* tRes = this;
        *tRes = static_cast<Resource&&> (std::move(rhs));
        if (this != &rhs) {
            shader = rhs.shader;
            type = rhs.type;
            strType = std::move(rhs.strType);
            rhs.shader = 0;
            rhs.type = GL_VERTEX_SHADER;
        }
        return *this;
    }

    /** Destructor. */
    Shader::~Shader()
    {
        if (IsLoaded()) UnloadLocal();
    }

    void Shader::Load()
    {
        shader = RecompileShader();
        Resource::Load();
    }

    /**
     * Reset the shader to a new name generated by RecompileShader before.
     * This is used to make sure an old shader is not lost if linking shaders to a program fails.
     * @param newShader the recompiled shader
     */
    void Shader::ResetShader(GLuint newShader)
    {
        Unload();
        shader = newShader;
        Resource::Load();
    }

    /**
     * Recompiles the shader.
     * The returned shader name should be set with ResetShader later after linking the program succeeded.
     * If the linking failed the program needs to delete the new shader object.
     * @return the new shader object name
     */
    GLuint Shader::RecompileShader() const
    {
        auto shaderDefinition = GetParameters();
        std::vector<std::string> defines(shaderDefinition.begin() + 1, shaderDefinition.end());
        return CompileShader(application->GetConfig().resourceBase + "/" + shaderDefinition[0], defines, type, strType);
    }

    void Shader::UnloadLocal()
    {
        if (this->shader != 0) {
            OGL_CALL(glDeleteShader, shader);
            shader = 0;
        }
    }

    void Shader::Unload()
    {
        UnloadLocal();
        Resource::Unload();
    }

    /**
     *  Loads a shader from file and recursively adds all includes.
     *  @param filename the name of the file to load.
     *  @param defines the defines to add at the beginning.
     *  @param fileId the id of the current file.
     */
    std::string Shader::LoadShaderFile(const std::string& filename, const std::vector<std::string>& defines, unsigned int& fileId, unsigned int recursionDepth) const
    {
        if (recursionDepth > 32) {
            LOG(ERROR) << L"Header inclusion depth limit reached! Cyclic header inclusion?";
            throw resource_loading_error() << ::boost::errinfo_file_name(filename) << fileid_info(fileId) << resid_info(id)
                << errdesc_info("Header inclusion depth limit reached! Cyclic header inclusion?");
        }
        boost::filesystem::path sdrFile{ filename };
        auto currentPath = sdrFile.parent_path().string() + "/";
        std::ifstream file(filename.c_str(), std::ifstream::in);
        std::string line;
        std::stringstream content;
        unsigned int lineCount = 1;
        auto nextFileId = fileId + 1;

        while (file.good()) {
            std::getline(file, line);
            auto trimedLine = line;
            boost::trim(trimedLine);

            static const boost::regex re("^[ ]*#[ ]*include[ ]+[\"<](.*)[\">].*");
            boost::smatch matches;
            if (boost::regex_search(line, matches, re)) {
                auto includeFile = currentPath + matches[1];
                if (!boost::filesystem::exists(includeFile)) {
                    LOG(ERROR) << filename.c_str() << L"(" << lineCount << ") : fatal error: cannot open include file \""
                        << includeFile.c_str() << "\".";
                    throw resource_loading_error() << ::boost::errinfo_file_name(includeFile) << fileid_info(fileId)
                        << lineno_info(lineCount - 1) << resid_info(id)
                        << errdesc_info("Cannot open include file.");
                }
                content << "#line " << 1 << " " << nextFileId << std::endl;
                content << LoadShaderFile(includeFile, std::vector<std::string>(), nextFileId, recursionDepth + 1);
                content << "#line " << lineCount + 1 << " " << fileId << std::endl;
            } else {
                content << line << std::endl;
            }

            if (boost::starts_with(trimedLine, "#version")) {
                for (auto& def : defines) {
                    auto trimedDefine = def;
                    boost::trim(trimedDefine);
                    content << "#define " << trimedDefine << std::endl;
                }
                content << "#line " << lineCount + 1 << " " << fileId << std::endl;
            }
            ++lineCount;
        }

        file.close();
        fileId = nextFileId;
        return content.str();
    }

    /**
     * Loads a shader from file and compiles it.
     * @param filename the shaders file name
     * @param type the shaders type
     * @param strType the shaders type as string
     * @return the compiled shader if successful
     */
    GLuint Shader::CompileShader(const std::string& filename, const std::vector<std::string>& defines, GLenum type, const std::string& strType) const
    {
        unsigned int firstFileId = 0;
        if (!boost::filesystem::exists(filename)) {
            LOG(ERROR) << "Cannot open shader file \"" << filename.c_str() << "\".";
            throw resource_loading_error() << ::boost::errinfo_file_name(filename) << fileid_info(firstFileId) << resid_info(id)
                << errdesc_info("Cannot open shader file.");
        }
        auto shaderText = LoadShaderFile(filename, defines, firstFileId, 0);

        auto shader = OGL_CALL(glCreateShader, type);
        if (shader == 0) {
            LOG(ERROR) << L"Could not create shader!";
            throw std::runtime_error("Could not create shader!");
        }
        auto shaderTextArray = shaderText.c_str();
        auto shaderLength = static_cast<int>(shaderText.length());
        OGL_CALL(glShaderSource, shader, 1, &shaderTextArray, &shaderLength);
        OGL_CALL(glCompileShader, shader);

        GLint status;
        OGL_CALL(glGetShaderiv, shader, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE) {
            GLint infoLogLength;
            OGL_CALL(glGetShaderiv, shader, GL_INFO_LOG_LENGTH, &infoLogLength);

            auto strInfoLog = new GLchar[infoLogLength + 1];
            OGL_CALL(glGetShaderInfoLog, shader, infoLogLength, NULL, strInfoLog);

            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            LOG(ERROR) << L"Compile failure in " << converter.from_bytes(strType) << L" shader ("
                << filename.c_str() << "): " << std::endl << strInfoLog;
            std::string infoLog = strInfoLog;
            delete[] strInfoLog;
            OGL_CALL(glDeleteShader, shader);
            throw shader_compiler_error() << ::boost::errinfo_file_name(filename)
                << compiler_error_info(infoLog) << resid_info(id)
                << errdesc_info("Shader compilation failed.");
        }
        return shader;
    }
}