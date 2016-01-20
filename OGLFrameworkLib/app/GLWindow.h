/**
 * @file   GLWindow.h
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2013.12.18
 * @ingroup win
 *
 * @brief  Windows declaration for the GLWindow.
 */

#ifndef GLWINDOW_H
#define GLWINDOW_H

#include "main.h"
#include "Configuration.h"
#include "BaseGLWindow.h"

namespace cgu {

    class ApplicationBase;

    /**
     * @brief Windows declaration for the GLWindow.
     * @ingroup win
     *
     * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
     * @date   2013.12.18
     */
    class GLWindow : public BaseGLWindow
    {
    public:
        GLWindow(HINSTANCE hInstance, int nCmdShow, const std::string& title, Configuration& conf);
        virtual ~GLWindow();

        void ShowWindow() const;
        void CloseWindow() override;
        void RegisterApplication(ApplicationBase& application);
        void Present() override;
        bool MessageBoxQuestion(const std::string& title, const std::string& content) override;

        LRESULT HandleMessages(UINT message, WPARAM wParam, LPARAM lParam);
        void HandleRawKeyboard(const RAWKEYBOARD& raw);
        void HandleCharInput(unsigned int key);
        void HandleRawMouse(const RAWMOUSE& raw);
        Configuration& GetConfig() const;
        HWND GetHWnd() const { return hWnd; }

    private:
        HWND hWnd;
        HDC hDC;
        HGLRC hRC;
        HINSTANCE instance;
        int cmdShow;
        std::string windowClass;
        std::string windowTitle;
        Configuration& config;
        ApplicationBase* app;

        // window status
        /// <summary>   true if rendering is paused. </summary>
        bool pause;
        /// <summary>   true if minimized. </summary>
        bool minimized;
        /// <summary>   true if maximized. </summary>
        bool maximized;
        /// <summary>   true if window is resizing. </summary>
        bool resizing;
        /// The number (id) of the current frame.
        unsigned int frameCount;

        void InitWindow();
        void InitOpenGL();
        void InitCUDA() const;
        void ReleaseWindow();
        void ReleaseOpenGL();
        void ReleaseCUDA() const;
        void HandleResize();
        void HandleSizeEvent(WPARAM wParam);
    };
}

#endif /* GLWINDOW_H */
