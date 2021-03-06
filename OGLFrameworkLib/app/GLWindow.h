/**
 * @file   GLWindow.h
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2013.12.18
 *
 * @brief  Windows declaration for the GLWindow.
 */

#ifndef GLWINDOW_H
#define GLWINDOW_H

#include "main.h"
#include "Configuration.h"
#include "gfx/glrenderer/GLRenderTarget.h"

struct GLFWwindow;

namespace cgu {

    class ApplicationBase;

    /**
     * @brief Windows declaration for the GLWindow.
     *
     * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
     * @date   2013.12.18
     */
    class GLWindow : public GLRenderTarget
    {
    public:
        GLWindow(const std::string& title, Configuration& conf);
        virtual ~GLWindow();

        bool IsClosing() const;
        void ShowWindow() const;
        void CloseWindow() const;
        void RegisterApplication(ApplicationBase& application);
        void Present() const;
        bool MessageBoxQuestion(const std::string& title, const std::string& content) const;

        Configuration& GetConfig() const { return config; };
        bool IsMouseButtonPressed(int button) const;
        bool IsKeyPressed(int key) const;

        /** Returns the windows width. */
        unsigned int GetWidth() const { return width; }
        /** Returns the windows height. */
        unsigned int GetHeight() const { return height; }
        /** Returns the windows client size. */
        glm::vec2 GetClientSize() const { return glm::vec2(static_cast<float>(width), static_cast<float>(height)); }
        glm::vec2 GetMousePosition() const { return currMousePosition_; }

    private:
        void WindowPosCallback(int xpos, int ypos) const;
        void WindowSizeCallback(int width, int height);
        void WindowFocusCallback(int focused) const;
        void WindowCloseCallback() const;
        void FramebufferSizeCallback(int width, int height) const;
        void WindowIconifyCallback(int iconified);

        void MouseButtonCallback(int button, int action, int mods);
        void CursorPosCallback(double xpos, double ypos);
        void CursorEnterCallback(int entered);
        void ScrollCallback(double xoffset, double yoffset);
        void KeyCallback(int key, int scancode, int action, int mods);
        void CharCallback(unsigned int codepoint) const;
        void CharModsCallback(unsigned int codepoint, int mods) const;
        void DropCallback(int count, const char** paths) const;

        /** Holds the GLFW window. */
        GLFWwindow* window_;
        /** Holds the current window size. */
        glm::vec2 windowSize_;
        std::string windowTitle;
        Configuration& config;
        ApplicationBase* app;

        /** Holds the current mouse position. */
        glm::vec2 currMousePosition_;
        /** Holds the last mouse position. */
        glm::vec2 prevMousePosition_;
        /** Holds the relative mouse position. */
        glm::vec2 relativeMousePosition_;
        /** Holds whether the mouse is inside the client window. */
        bool mouseInWindow_;

        // window status
        /** <c>true</c> if minimized. */
        bool minimized;
        /** <c>true</c> if maximized. */
        bool maximized;
        /** The number (id) of the current frame. */
        unsigned int frameCount;

        void InitWindow();
        void InitOpenGL();
        void InitCUDA() const;
        void ReleaseWindow();
        void ReleaseOpenGL();
        void ReleaseCUDA() const;

    public:
        static void glfwErrorCallback(int error, const char* description);
        static void glfwWindowPosCallback(GLFWwindow* window, int xpos, int ypos);
        static void glfwWindowSizeCallback(GLFWwindow* window, int width, int height);
        static void glfwWindowFocusCallback(GLFWwindow* window, int focused);
        static void glfwWindowCloseCallback(GLFWwindow* window);
        static void glfwFramebufferSizeCallback(GLFWwindow* window, int width, int height);
        static void glfwWindowIconifyCallback(GLFWwindow* window, int iconified);

        static void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
        static void glfwCursorPosCallback(GLFWwindow* window, double xpos, double ypos);
        static void glfwCursorEnterCallback(GLFWwindow* window, int entered);
        static void glfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
        static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void glfwCharCallback(GLFWwindow* window, unsigned int codepoint);
        static void glfwCharModsCallback(GLFWwindow* window, unsigned int codepoint, int mods);
        static void glfwDropCallback(GLFWwindow* window, int count, const char** paths);
    };
}

#endif /* GLWINDOW_H */
