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

        typename T::value_type operator[](size_t i) { return objs[i]; }

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
        typename T::value_type get() { return obj; }
        operator bool() { return T::null_obj != obj; }
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

    using ProgramRAII = OpenGLRAIIWrapper<ProgramObjectTraits, 1>;
    using ShaderRAII = OpenGLRAIIWrapper<ShaderObjectTraits, 1>;
    template<int N> using BuffersRAII = OpenGLRAIIWrapper<BufferObjectTraits, N>;
    using BufferRAII = OpenGLRAIIWrapper<BufferObjectTraits, 1>;
}

