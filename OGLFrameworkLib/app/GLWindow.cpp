/**
 * @file   WinGLWindow.cpp
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2013.12.18
 *
 * @brief  Windows implementation for the GLWindow.
 */

#ifdef _WIN32

#include "GLWindow.h"
#include "gfx/glrenderer/FrameBuffer.h"
#include "ApplicationBase.h"

#include <cuda_runtime_api.h>
#include <cuda_gl_interop.h>
#include <codecvt>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>

namespace cgu {

    /**
     * Logs the debug output of OpenGL.
     * @param source the source of the debug message
     * @param type the type of debug message
     * @param severity the severity of the debug message
     * @param message the debug message
     */
    void WINAPI DebugOutputCallback(GLenum source, GLenum type, GLuint, GLenum severity, GLsizei, const GLchar* message, const void*)
    {
        std::stringstream str;
        str << "OpenGL Debug Output message : ";

        if (source == GL_DEBUG_SOURCE_API_ARB) str << "Source : API; ";
        else if (source == GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB) str << "Source : WINDOW_SYSTEM; ";
        else if (source == GL_DEBUG_SOURCE_SHADER_COMPILER_ARB) str << "Source : SHADER_COMPILER; ";
        else if (source == GL_DEBUG_SOURCE_THIRD_PARTY_ARB) str << "Source : THIRD_PARTY; ";
        else if (source == GL_DEBUG_SOURCE_APPLICATION_ARB) str << "Source : APPLICATION; ";
        else if (source == GL_DEBUG_SOURCE_OTHER_ARB) str << "Source : OTHER; ";

        if (type == GL_DEBUG_TYPE_ERROR_ARB) {
            str << "Type : ERROR; ";
        }
        else if (type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB) str << "Type : DEPRECATED_BEHAVIOR; ";
        else if (type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB) str << "Type : UNDEFINED_BEHAVIOR; ";
        else if (type == GL_DEBUG_TYPE_PORTABILITY_ARB) str << "Type : PORTABILITY; ";
        else if (type == GL_DEBUG_TYPE_PERFORMANCE_ARB) str << "Type : PERFORMANCE; ";
        else if (type == GL_DEBUG_TYPE_OTHER_ARB) str << "Type : OTHER; ";

        if (severity == GL_DEBUG_SEVERITY_HIGH_ARB) str << "Severity : HIGH; ";
        else if (severity == GL_DEBUG_SEVERITY_MEDIUM_ARB) str << "Severity : MEDIUM; ";
        else if (severity == GL_DEBUG_SEVERITY_LOW_ARB) str << "Severity : LOW; ";

        // You can set a breakpoint here ! Your debugger will stop the program,
        // and the call stack will immediately show you the offending call.
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        LOG(GL_DEBUG) << converter.from_bytes(str.str()) << "Message : " << message;
    }

    /**
     * Creates a new windows GLWindow.
     * @param title the windows title.
     * @param conf the configuration used
     */
    GLWindow::GLWindow(const std::string& title, Configuration& conf) :
        GLRenderTarget(conf.windowWidth, conf.windowHeight),
        window_{ nullptr },
        windowTitle(title),
        config(conf),
        app(nullptr),
        currMousePosition_(0.0f),
        prevMousePosition_(0.0f),
        relativeMousePosition_(0.0f),
        mouseInWindow_(true),
        minimized(false),
        maximized(conf.fullscreen),
        frameCount(0)
    {
        this->InitWindow();
        this->InitOpenGL();
        if (config.useCUDA) this->InitCUDA();
    }

    GLWindow::~GLWindow()
    {
        if (config.useCUDA) this->ReleaseCUDA();
        this->ReleaseOpenGL();
        this->ReleaseWindow();
        config.fullscreen = maximized;
        config.windowWidth = fbo.GetWidth();
        config.windowHeight = fbo.GetHeight();
    }

    bool GLWindow::IsClosing() const
    {
        return glfwWindowShouldClose(window_) == GLFW_TRUE;
    }

    /**
     * Initializes the window.
     */
    void GLWindow::InitWindow()
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, PTRN_OPENGL_MAJOR_VERSION);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, PTRN_OPENGL_MINOR_VERSION);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#ifdef _DEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
        glfwWindowHint(GLFW_RED_BITS, 8);
        glfwWindowHint(GLFW_GREEN_BITS, 8);
        glfwWindowHint(GLFW_BLUE_BITS, 8);
        glfwWindowHint(GLFW_ALPHA_BITS, 0);
        if (config.backbufferBits == 32) {
            glfwWindowHint(GLFW_DEPTH_BITS, 32);
            glfwWindowHint(GLFW_STENCIL_BITS, 0);
        } else {
            glfwWindowHint(GLFW_DEPTH_BITS, config.backbufferBits);
        }
        glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_SAMPLES, 8);

        glfwSetErrorCallback(GLWindow::glfwErrorCallback);
        if (config.fullscreen) {
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
            window_ = glfwCreateWindow(config.windowWidth, config.windowHeight, windowTitle.c_str(), glfwGetPrimaryMonitor(), nullptr);
            if (window_ == nullptr) {
                LOG(ERROR) << L"Could not create window!";
                glfwTerminate();
                throw std::runtime_error("Could not create window!");
            }
            glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
            glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
            window_ = glfwCreateWindow(config.windowWidth, config.windowHeight, windowTitle.c_str(), nullptr, nullptr);
            if (window_ == nullptr) {
                LOG(ERROR) << L"Could not create window!";
                glfwTerminate();
                throw std::runtime_error("Could not create window!");
            }
            glfwSetWindowPos(window_, config.windowLeft, config.windowTop);
            
        }
        glfwSetWindowUserPointer(window_, this);
        glfwSetInputMode(window_, GLFW_STICKY_MOUSE_BUTTONS, 1);
        glfwSetCursorPos(window_, 0.0, 0.0);

        glfwSetWindowPosCallback(window_, GLWindow::glfwWindowPosCallback);
        glfwSetWindowSizeCallback(window_, GLWindow::glfwWindowSizeCallback);
        glfwSetWindowFocusCallback(window_, GLWindow::glfwWindowFocusCallback);
        glfwSetWindowCloseCallback(window_, GLWindow::glfwWindowCloseCallback);
        glfwSetFramebufferSizeCallback(window_, GLWindow::glfwFramebufferSizeCallback);
        glfwSetWindowIconifyCallback(window_, GLWindow::glfwWindowIconifyCallback);


        glfwSetMouseButtonCallback(window_, GLWindow::glfwMouseButtonCallback);
        glfwSetCursorPosCallback(window_, GLWindow::glfwCursorPosCallback);
        glfwSetCursorEnterCallback(window_, GLWindow::glfwCursorEnterCallback);
        glfwSetScrollCallback(window_, GLWindow::glfwScrollCallback);
        glfwSetKeyCallback(window_, GLWindow::glfwKeyCallback);
        glfwSetCharCallback(window_, GLWindow::glfwCharCallback);
        glfwSetCharModsCallback(window_, GLWindow::glfwCharModsCallback);
        glfwSetDropCallback(window_, GLWindow::glfwDropCallback);

        // Check for Valid Context
        if (window_ == nullptr) {
            LOG(ERROR) << L"Could not create window!";
            glfwTerminate();
            throw std::runtime_error("Could not create window!");
        }

        LOG(DEBUG) << L"Window successfully initialized.";
    }

    /**
     * Initializes OpenGL.
     */
    void GLWindow::InitOpenGL()
    {
        LOG(INFO) << L"Initializing OpenGL context...";
        glfwMakeContextCurrent(window_);

        LOG(INFO) << L"Initializing glad...";
        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
            this->ReleaseOpenGL();
            LOG(ERROR) << L"Could not initialize glad!";
            throw std::runtime_error("Could not initialize glad!");
        }

        // TODO: higher OpenGL version?
        LOG(INFO) << L"Checking OpenGL version 4.0 ...";
        if (!GLAD_GL_VERSION_4_0) {
            this->ReleaseOpenGL();
            LOG(ERROR) << L"OpenGL version not supported.";
            throw std::runtime_error("OpenGL version not supported.");
        }


#ifdef _DEBUG
        if (GLAD_GL_ARB_debug_output) {
            LOG(DEBUG) << L"The OpenGL implementation provides debug output.";
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(&DebugOutputCallback, nullptr);
            GLuint unusedIds = 0;
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &unusedIds, true);
            glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION_ARB, GL_DEBUG_TYPE_OTHER_ARB,
                1, GL_DEBUG_SEVERITY_HIGH, -1, "OpenGL Debug Log here ...");
        } else {
            LOG(DEBUG) << L"The OpenGL implementation does not provide debug output.";
        }
#endif

        fbo.Resize(config.windowWidth, config.windowHeight);

        if (!GLAD_GL_ARB_vertex_array_object) {
            LOG(WARNING) << L"VAOs not supported ...";
        }

        if (config.useSRGB) {
            glEnable(GL_FRAMEBUFFER_SRGB);
        }

        LOG(INFO) << L"OpenGL context initialized.";

        ImGui_ImplGlfwGL3_Init(window_, false);
    }

    void GLWindow::InitCUDA() const
    {
        cudaError_t result;
        if (config.cudaDevice == -1) {
            unsigned int deviceCount;
            int devices;
            result = cudaGLGetDevices(&deviceCount, &devices, 1, cudaGLDeviceListAll);
            if (result != cudaSuccess) {
                LOG(ERROR) << L"Could not get CUDA GL devices. Error: " << result;
                throw std::runtime_error("Could not get CUDA GL devices.");
            }

            if (deviceCount == 0) {
                LOG(ERROR) << L"No CUDA devices found.";
                throw std::runtime_error("No CUDA devices found.");
            }
            config.cudaDevice = devices;
        }

        result = cudaSetDevice(config.cudaDevice);
        if (result != cudaSuccess) {
            LOG(ERROR) << L"Could not set CUDA device. Error: " << result;
            config.cudaDevice = -1;
            throw std::runtime_error("Could not set CUDA device.");
        }
        cudaDeviceReset();
        cudaDeviceSynchronize();
    }

    /**
     * Registers the application object using the window for event management.
     * @param application the application object
     */
    void GLWindow::RegisterApplication(ApplicationBase & application)
    {
        this->app = &application;
    }

    /**
     * Shows the window.
     */
    void GLWindow::ShowWindow() const
    {
        glfwShowWindow(window_);
    }

    /**
     *  Closes the window.
     */
    void GLWindow::CloseWindow() const
    {
        glfwSetWindowShouldClose(window_, GLFW_TRUE);
    }

    // ReSharper disable once CppMemberFunctionMayBeStatic
    void GLWindow::ReleaseCUDA() const
    {
        auto result = cudaDeviceReset();
        if (result != cudaSuccess) {
            LOG(ERROR) << L"Could not reset CUDA device. Error: " << result;
        }
    }

    // ReSharper disable once CppMemberFunctionMayBeStatic
    // ReSharper disable once CppMemberFunctionMayBeConst
    void GLWindow::ReleaseOpenGL()
    {
        ImGui_ImplGlfwGL3_Shutdown();
    }

    void GLWindow::ReleaseWindow()
    {
        if (window_) glfwDestroyWindow(window_);
        window_ = nullptr;
    }

    /**
     * Swaps buffers and shows the content rendered since last call of Present().
     */
    void GLWindow::Present() const
    {
        glfwSwapBuffers(window_);
    }

    /**
     * Shows a question message box.
     * @param title the message boxes title
     * @param content the message boxes content
     * @return returns <code>true</code> if the user pressed 'yes' <code>false</code> if 'no'
     */
    bool GLWindow::MessageBoxQuestion(const std::string& title, const std::string& content) const
    {
        return MessageBoxA(glfwGetWin32Window(window_), content.c_str(), title.c_str(), MB_YESNO) == IDYES;
    }

    bool GLWindow::IsMouseButtonPressed(int button) const
    {
        return glfwGetMouseButton(window_, button) == GLFW_PRESS;
    }

    void GLWindow::WindowPosCallback(int xpos, int ypos) const
    {
        config.windowLeft = xpos;
        config.windowTop = ypos;
    }

    void GLWindow::WindowSizeCallback(int width, int height)
    {
        assert(this->app != nullptr);
        LOG(DEBUG) << L"Begin HandleResize()";

        if (width == 0 || height == 0) {
            return;
        }
        this->Resize(width, height);

        try {
            this->app->OnResize(width, height);
        }
        catch (std::runtime_error e) {
            LOG(ERROR) << L"Could not reacquire resources after resize: " << e.what();
            throw std::runtime_error("Could not reacquire resources after resize.");
        }

        this->config.windowWidth = width;
        this->config.windowHeight = height;
    }

    void GLWindow::WindowFocusCallback(int focused) const
    {
        if (app != nullptr) {
            if (focused == GLFW_TRUE) app->SetPause(false);
            else if (GetConfig().pauseOnKillFocus) app->SetPause(true);
        }
    }

    // ReSharper disable once CppMemberFunctionMayBeStatic
    void GLWindow::WindowCloseCallback() const
    {
        LOG(INFO) << L"Got close event ...";
    }

    // ReSharper disable once CppMemberFunctionMayBeStatic
    void GLWindow::FramebufferSizeCallback(int width, int height) const
    {
        LOG(INFO) << L"Got framebuffer resize event (" << width << ", " << height << ") ...";
    }

    void GLWindow::WindowIconifyCallback(int iconified)
    {
        if (iconified == GLFW_TRUE) {
            if (app != nullptr) app->SetPause(true);
            minimized = true;
            maximized = false;
        } else {
            if (minimized && app != nullptr) app->SetPause(false);
            minimized = false;
            maximized = false;
        }
    }

    void GLWindow::MouseButtonCallback(int button, int action, int mods)
    {
        if (mouseInWindow_ && app != nullptr) {
            app->HandleMouse(button, action, mods, 0.0f, this);
        }
    }

    void GLWindow::CursorPosCallback(double xpos, double ypos)
    {
        if (mouseInWindow_) {
            prevMousePosition_ = currMousePosition_;
            currMousePosition_ = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
            relativeMousePosition_ = currMousePosition_ - prevMousePosition_;

            if (app != nullptr) {
                app->HandleMouse(-1, 0, 0, 0.0f, this);
            }
        }
    }

    void GLWindow::CursorEnterCallback(int entered)
    {
        if (entered) {
            double xpos, ypos;
            glfwGetCursorPos(window_, &xpos, &ypos);
            currMousePosition_ = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
            mouseInWindow_ = true;
        } else {
            mouseInWindow_ = false;
        }
    }

    void GLWindow::ScrollCallback(double, double yoffset)
    {
        if (mouseInWindow_ && app != nullptr) {
            app->HandleMouse(-1, 0, 0, static_cast<float>(yoffset), this);
        }
    }

    void GLWindow::KeyCallback(int key, int scancode, int action, int mods)
    {
        if (app != nullptr) {
            this->app->HandleKeyboard(key, scancode, action, mods, this);
        }
    }

    // ReSharper disable once CppMemberFunctionMayBeStatic
    void GLWindow::CharCallback(unsigned) const
    {
        // Not needed at this point... [4/7/2016 Sebastian Maisch]
    }

    // ReSharper disable once CppMemberFunctionMayBeStatic
    void GLWindow::CharModsCallback(unsigned, int) const
    {
        // Not needed at this point... [4/7/2016 Sebastian Maisch]
    }

    // ReSharper disable once CppMemberFunctionMayBeStatic
    void GLWindow::DropCallback(int, const char**) const
    {
        throw std::runtime_error("File dropping not implemented.");
    }

    void GLWindow::glfwErrorCallback(int error, const char* description)
    {
        LOG(ERROR) << "An GLFW error occurred (" << error << "): " << std::endl << description;
    }

    void GLWindow::glfwWindowPosCallback(GLFWwindow* window, int xpos, int ypos)
    {
        auto win = reinterpret_cast<GLWindow*>(glfwGetWindowUserPointer(window));
        win->WindowPosCallback(xpos, ypos);
    }

    void GLWindow::glfwWindowSizeCallback(GLFWwindow* window, int width, int height)
    {
        auto win = reinterpret_cast<GLWindow*>(glfwGetWindowUserPointer(window));
        win->WindowSizeCallback(width, height);
    }

    void GLWindow::glfwWindowFocusCallback(GLFWwindow* window, int focused)
    {
        auto win = reinterpret_cast<GLWindow*>(glfwGetWindowUserPointer(window));
        win->WindowFocusCallback(focused);
    }

    void GLWindow::glfwWindowCloseCallback(GLFWwindow* window)
    {
        auto win = reinterpret_cast<GLWindow*>(glfwGetWindowUserPointer(window));
        win->WindowCloseCallback();
    }

    void GLWindow::glfwFramebufferSizeCallback(GLFWwindow* window, int width, int height)
    {
        auto win = reinterpret_cast<GLWindow*>(glfwGetWindowUserPointer(window));
        win->FramebufferSizeCallback(width, height);
    }

    void GLWindow::glfwWindowIconifyCallback(GLFWwindow* window, int iconified)
    {
        auto win = reinterpret_cast<GLWindow*>(glfwGetWindowUserPointer(window));
        win->WindowIconifyCallback(iconified);
    }

    void GLWindow::glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        ImGui_ImplGlfwGL3_MouseButtonCallback(window, button, action, mods);

        auto& io = ImGui::GetIO();
        if (!io.WantCaptureMouse) {
            auto win = reinterpret_cast<GLWindow*>(glfwGetWindowUserPointer(window));
            win->MouseButtonCallback(button, action, mods);
        }
    }

    void GLWindow::glfwCursorPosCallback(GLFWwindow* window, double xpos, double ypos)
    {
        auto& io = ImGui::GetIO();
        if (!io.WantCaptureMouse) {
            auto win = reinterpret_cast<GLWindow*>(glfwGetWindowUserPointer(window));
            win->CursorPosCallback(xpos, ypos);
        }
    }

    void GLWindow::glfwCursorEnterCallback(GLFWwindow* window, int entered)
    {
        auto win = reinterpret_cast<GLWindow*>(glfwGetWindowUserPointer(window));
        win->CursorEnterCallback(entered);
    }

    void GLWindow::glfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
        ImGui_ImplGlfwGL3_ScrollCallback(window, xoffset, yoffset);

        auto& io = ImGui::GetIO();
        if (!io.WantCaptureMouse) {
            auto win = reinterpret_cast<GLWindow*>(glfwGetWindowUserPointer(window));
            win->ScrollCallback(xoffset, yoffset);
        }
    }

    void GLWindow::glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        ImGui_ImplGlfwGL3_KeyCallback(window, key, scancode, action, mods);

        auto& io = ImGui::GetIO();
        if (!io.WantCaptureKeyboard) {
            auto win = reinterpret_cast<GLWindow*>(glfwGetWindowUserPointer(window));
            win->KeyCallback(key, scancode, action, mods);
        }
    }

    void GLWindow::glfwCharCallback(GLFWwindow* window, unsigned codepoint)
    {
        ImGui_ImplGlfwGL3_CharCallback(window, codepoint);

        auto& io = ImGui::GetIO();
        if (!io.WantCaptureKeyboard) {
            auto win = reinterpret_cast<GLWindow*>(glfwGetWindowUserPointer(window));
            win->CharCallback(codepoint);
        }
    }

    void GLWindow::glfwCharModsCallback(GLFWwindow* window, unsigned codepoint, int mods)
    {
        auto& io = ImGui::GetIO();
        if (!io.WantCaptureKeyboard) {
            auto win = reinterpret_cast<GLWindow*>(glfwGetWindowUserPointer(window));
            win->CharModsCallback(codepoint, mods);
        }
    }

    void GLWindow::glfwDropCallback(GLFWwindow* window, int count, const char** paths)
    {
        auto win = reinterpret_cast<GLWindow*>(glfwGetWindowUserPointer(window));
        win->DropCallback(count, paths);
    }
}

#endif /* _WIN32 */
