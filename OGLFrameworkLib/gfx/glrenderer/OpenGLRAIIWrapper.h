/**
 * @file   OpenGLRAIIWrapper.h
 * @author Sebastian Maisch <sebastian.maisch@googlemailcom>
 * @date   2016.01.24
 *
 * @brief  Wrappers for OpenGL object to allow RAII patterns.
 */

#pragma once

// ReSharper disable once CppUnusedIncludeDirective
#include "main.h"

namespace cgu {

    template<typename T, int N>
    class OpenGLRAIIWrapper
    {
    public:
        OpenGLRAIIWrapper() { T::Create<N>(objs); }
        OpenGLRAIIWrapper(const OpenGLRAIIWrapper&) = delete;
        OpenGLRAIIWrapper& operator=(const OpenGLRAIIWrapper&) = delete;
        OpenGLRAIIWrapper(OpenGLRAIIWrapper&& rhs) : objs(std::move(rhs.objs)) { for (auto& obj : rhs.objs) obj = T::null_obj; }
        OpenGLRAIIWrapper& operator=(OpenGLRAIIWrapper&& rhs) { objs = std::move(rhs.objs); for (auto& obj : rhs.objs) obj = T::null_obj; return *this; }
        ~OpenGLRAIIWrapper() { T::Destroy<N>(objs); }

        typename T::value_type operator[](size_t i) const { return objs[i]; }

    private:
        std::array<typename T::value_type, N> objs;
    };

    template<typename T>
    class OpenGLRAIIWrapper<T, 1>
    {
    public:
        OpenGLRAIIWrapper() : obj(T::Create()) {}
        explicit OpenGLRAIIWrapper(typename T::value_type newObj) : obj(newObj) {}
        OpenGLRAIIWrapper(const OpenGLRAIIWrapper&) = delete;
        OpenGLRAIIWrapper& operator=(const OpenGLRAIIWrapper&) = delete;
        OpenGLRAIIWrapper(OpenGLRAIIWrapper&& rhs) : obj(rhs.obj) { rhs.obj = T::null_obj; }
        OpenGLRAIIWrapper& operator=(OpenGLRAIIWrapper&& rhs) { obj = rhs.obj; rhs.obj = T::null_obj; return *this; }
        ~OpenGLRAIIWrapper() { obj = T::Destroy(obj); }

        operator typename T::value_type() const { return obj; }
        // typename T::value_type get() const { return obj; }
        explicit operator bool() const { return T::null_obj != obj; }
        bool operator==(const OpenGLRAIIWrapper& rhs) { return rhs.obj == obj; }

        friend bool operator==(typename T::value_type lhs, const OpenGLRAIIWrapper<T, 1>& rhs) { return lhs == rhs.obj; }
        friend bool operator==(const OpenGLRAIIWrapper<T, 1>& lhs, typename T::value_type rhs) { return lhs.obj == rhs; }

        typename T::value_type release() { typename T::value_type tmp = obj; obj = T::null_obj; return tmp; }
        void reset(typename T::value_type newObj = T::null_obj) { obj = T::Destroy(obj); obj = newObj; }
        void swap(OpenGLRAIIWrapper<T, 1>& other) { typename T::value_type tmp = obj; obj = other.obj; other.obj = tmp; }

    private:
        typename T::value_type obj;
    };

    struct ProgramObjectTraits
    {
        using value_type = GLuint;
        static const value_type null_obj = 0;
        static value_type Create() { return OGL_SCALL(glCreateProgram); }
        static value_type Destroy(value_type prog) { OGL_CALL(glDeleteProgram, prog); return null_obj; }
    };

    struct ShaderObjectTraits
    {
        using value_type = GLuint;
        static const value_type null_obj = 0;
        static value_type Create() { return null_obj; }
        static value_type Destroy(value_type shader) { OGL_CALL(glDeleteShader, shader); return null_obj; }
    };

    struct BufferObjectTraits
    {
        using value_type = GLuint;
        static const value_type null_obj = 0;
        static value_type Create() { value_type buffer; OGL_CALL(glGenBuffers, 1, &buffer); return buffer; }
        template<int N> static void Create(std::array<value_type, N>& buffers) { OGL_CALL(glGenBuffers, static_cast<GLsizei>(N), buffers.data()); }
        static value_type Destroy(value_type buffer) { OGL_CALL(glDeleteBuffers, 1, &buffer); return null_obj; }
        template<int N> static void Destroy(std::array<value_type, N>& buffers)
        {
            OGL_CALL(glDeleteBuffers, static_cast<GLsizei>(N), buffers.data());
            for (auto& buffer : buffers) buffer = null_obj;
        }
    };

    struct TextureObjectTraits
    {
        using value_type = GLuint;
        static const value_type null_obj = 0;
        static value_type Create() { value_type texture; OGL_CALL(glGenTextures, 1, &texture); return texture; }
        template<int N> static void Create(std::array<value_type, N>& textures) { OGL_CALL(glGenTextures, static_cast<GLsizei>(N), textures.data()); }
        static value_type Destroy(value_type texture) { OGL_CALL(glDeleteTextures, 1, &texture); return null_obj; }
        template<int N> static void Destroy(std::array<value_type, N>& textures)
        {
            OGL_CALL(glDeleteTextures, static_cast<GLsizei>(N), textures.data());
            for (auto& texture : textures) texture = null_obj;
        }
    };

    struct FramebufferObjectTraits
    {
        using value_type = GLuint;
        static const value_type null_obj = 0;
        static value_type Create() { value_type fbo; OGL_CALL(glGenFramebuffers, 1, &fbo); return fbo; }
        template<int N> static void Create(std::array<value_type, N>& fbos) { OGL_CALL(glGenFramebuffers, static_cast<GLsizei>(N), fbos.data()); }
        static value_type Destroy(value_type fbo) { OGL_CALL(glDeleteFramebuffers, 1, &fbo); return null_obj; }
        template<int N> static void Destroy(std::array<value_type, N>& fbos)
        {
            OGL_CALL(glDeleteFramebuffers, static_cast<GLsizei>(N), fbos.data());
            for (auto& fbo : fbos) fbo = null_obj;
        }
    };

    struct RenderbufferObjectTraits
    {
        using value_type = GLuint;
        static const value_type null_obj = 0;
        static value_type Create() { value_type rbo; OGL_CALL(glGenRenderbuffers, 1, &rbo); return rbo; }
        template<int N> static void Create(std::array<value_type, N>& rbos) { OGL_CALL(glGenRenderbuffers, static_cast<GLsizei>(N), rbos.data()); }
        static value_type Destroy(value_type rbo) { OGL_CALL(glDeleteRenderbuffers, 1, &rbo); return null_obj; }
        template<int N> static void Destroy(std::array<value_type, N>& rbos)
        {
            OGL_CALL(glDeleteRenderbuffers, static_cast<GLsizei>(N), rbos.data());
            for (auto& rbo : rbos) rbo = null_obj;
        }
    };

    struct VertexArrayObjectTraits
    {
        using value_type = GLuint;
        static const value_type null_obj = 0;
        static value_type Create() { value_type vao; OGL_CALL(glGenVertexArrays, 1, &vao); return vao; }
        template<int N> static void Create(std::array<value_type, N>& vaos) { OGL_CALL(glGenVertexArrays, static_cast<GLsizei>(N), vaos.data()); }
        static value_type Destroy(value_type vao) { OGL_CALL(glDeleteVertexArrays, 1, &vao); return null_obj; }
        template<int N> static void Destroy(std::array<value_type, N>& vaos)
        {
            OGL_CALL(glDeleteVertexArrays, static_cast<GLsizei>(N), vaos.data());
            for (auto& vao : vaos) vao = null_obj;
        }
    };

    using ProgramRAII = OpenGLRAIIWrapper<ProgramObjectTraits, 1>;
    using ShaderRAII = OpenGLRAIIWrapper<ShaderObjectTraits, 1>;
    template<int N> using BuffersRAII = OpenGLRAIIWrapper<BufferObjectTraits, N>;
    using BufferRAII = OpenGLRAIIWrapper<BufferObjectTraits, 1>;
    template<int N> using TexuturesRAII = OpenGLRAIIWrapper<TextureObjectTraits, N>;
    using TextureRAII = OpenGLRAIIWrapper<TextureObjectTraits, 1>;
    template<int N> using FramebuffersRAII = OpenGLRAIIWrapper<FramebufferObjectTraits, N>;
    using FramebufferRAII = OpenGLRAIIWrapper<FramebufferObjectTraits, 1>;
    template<int N> using RenderbuffersRAII = OpenGLRAIIWrapper<RenderbufferObjectTraits, N>;
    using RenderbufferRAII = OpenGLRAIIWrapper<RenderbufferObjectTraits, 1>;
    template<int N> using VertexArraysRAII = OpenGLRAIIWrapper<VertexArrayObjectTraits, N>;
    using VertexArrayRAII = OpenGLRAIIWrapper<VertexArrayObjectTraits, 1>;
}

