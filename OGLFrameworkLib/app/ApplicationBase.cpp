/**
 * @file   ApplicationBase.cpp
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2013.12.17
 *
 * @brief  Implements the application base class.
 */

#include "ApplicationBase.h"
#include "gfx/OrthogonalView.h"
#include "core/FontManager.h"
#include "app/GLWindow.h"
#include "gfx/glrenderer/ScreenQuadRenderable.h"
#include "app/gui/imgui_impl_gl3.h"

namespace cgu {
    /**
     * Construct a new application.
     * @param window the applications main window
     */
    ApplicationBase::ApplicationBase(GLWindow& window, const glm::vec3& camPos) :
        m_pause(true),
        m_stopped(false),
        m_time(0.0),
        m_elapsedTime(0.0),
        m_QPFTicksPerSec(0),
        m_lastElapsedTime(0),
        m_baseTime(0),
        m_currentScene(0),
        win(window),
        texManager(),
        matManager(),
        shaderManager(),
        programManager(),
        fontManager(),
        uniformBindingPoints(),
        shaderStorageBindingPoints(),
        orthoView(),
        cameraView(),
        fontProgram(nullptr),
        screenQuadRenderable(nullptr)
    {
        texManager.reset(new TextureManager(this));
        volManager.reset(new VolumeManager(this));
        matManager.reset(new MaterialLibManager(this));
        shaderManager.reset(new ShaderManager(this));
        programManager.reset(new GPUProgramManager(this));
        fontManager.reset(new FontManager(this));
        win.RegisterApplication(*this);
        win.ShowWindow();
        glm::vec2 screenSize(static_cast<float> (win.GetWidth()), static_cast<float> (win.GetHeight()));
        orthoView.reset(new OrthogonalView(screenSize, &uniformBindingPoints));
        cameraView.reset(new ArcballCamera(60.0f, screenSize, 1.0f, 100.0f, camPos, &uniformBindingPoints));

        imguiImpl::ImGui_ImplGL3_Init(win.GetHWnd());
        fontProgram = programManager->GetResource(fontProgramID);
        fontProgram->BindUniformBlock(orthoProjectionUBBName, uniformBindingPoints);
        screenQuadRenderable.reset(new ScreenQuadRenderable());
    }

    ApplicationBase::~ApplicationBase()
    {
        imguiImpl::ImGui_ImplGL3_Shutdown();
    }

    /**
     * Returns the texture manager.
     * @return  the texture manager
     */
    TextureManager* ApplicationBase::GetTextureManager() const
    {
        return texManager.get();
    }

    /**
     * Returns the volume manager.
     * @return  the volume manager
     */
    VolumeManager* ApplicationBase::GetVolumeManager() const
    {
        return volManager.get();
    }

    /**
     * Returns the material lib manager.
     * @return the material lib manager
     */
    MaterialLibManager* ApplicationBase::GetMaterialLibManager() const
    {
        return matManager.get();
    }

    /**
     * Returns the shader manager.
     * @return the shader manager
     */
    ShaderManager* ApplicationBase::GetShaderManager() const
    {
        return shaderManager.get();
    }

    /**
     * Returns the GPU program manager.
     * @return the GPU program manager
     */
    GPUProgramManager* ApplicationBase::GetGPUProgramManager() const
    {
        return programManager.get();
    }

    /**
     * Returns the uniform buffer binding points
     * @return the ubo binding points
     */
    ShaderBufferBindingPoints* ApplicationBase::GetUBOBindingPoints()
    {
        return &uniformBindingPoints;
    }

    /**
    * Returns the shader storage buffer object binding points
    * @return the ssbo binding points
    */
    ShaderBufferBindingPoints* ApplicationBase::GetSSBOBindingPoints()
    {
        return &shaderStorageBindingPoints;
    }

    /**
     * Returns the current configuration.
     * @return the configuration
     */
    Configuration& ApplicationBase::GetConfig() const
    {
        return win.GetConfig();
    }

    /**
     * Returns the font manager.
     * @return the font manager
     */
    FontManager* ApplicationBase::GetFontManager() const
    {
        return fontManager.get();
    }

    /**
     * Returns the main window.
     * @return the main window
     */
    GLWindow* ApplicationBase::GetWindow() const
    {
        return &win;
    }

    /**
     * Returns the GPU program for font rendering.
     * @return the font rendering program
     */
    std::shared_ptr<GPUProgram> ApplicationBase::GetFontProgram() const
    {
        return fontProgram;
    }

    /**
     *  Returns the screen quad renderable.
     *  @return the screen quad renderable.
     */
    ScreenQuadRenderable* ApplicationBase::GetScreenQuadRenderable() const
    {
        return screenQuadRenderable.get();
    }

    /**
     * Handles all keyboard input.
     * @param vkCode the key pressed
     * @param bKeyDown <code>true</code> if the key is pressed, else it is released
     * @param sender the window that send the keyboard messages
     */
    bool ApplicationBase::HandleKeyboard(unsigned int vkCode, bool bKeyDown, BaseGLWindow* sender)
    {
        auto handled = imguiImpl::ImGui_ImplGL3_HandleKey(vkCode, bKeyDown);

        if (bKeyDown && !handled) {
            switch (vkCode)
            {
            case VK_ESCAPE:
                this->win.CloseWindow();
                handled = 1;
                break;
            case VK_F9:
                this->programManager->RecompileAll();
                handled = 1;
                break;
            }
        }

        if (!handled && IsRunning() && !IsPaused()) handled = cameraView->HandleKeyboard(vkCode, bKeyDown, sender);

        return handled;
    }

    bool ApplicationBase::HandleKeyboardCharacters(unsigned key, BaseGLWindow*)
    {
        return imguiImpl::ImGui_ImplGL3_HandleChar(key);
        /*switch (key)
        {
        case 0x08: // Process a backspace. 
            break;
        case 0x0A: // Process a linefeed.
            break;
        case 0x1B: // Process an escape. 
            break;
        case 0x09: // Process a tab. 
            break;
        case 0x0D: // Process a carriage return.
            break;
        default: // Process displayable characters.
            break;
        }*/
    }

    /**
     * Handles mouse input.
     * @param buttonAction mouse action flags, see RAWMOUSE.usButtonFlags
     * @param mouseWheelDelta the scroll wheel delta
     * @param sender the window that send the keyboard messages
     * @return whether the message was handled
     */
    bool ApplicationBase::HandleMouse(unsigned int buttonAction, float mouseWheelDelta, BaseGLWindow* sender)
    {
        auto handledMovement = false;
        if (sender->HadPositionUpdate()) {
            handledMovement = imguiImpl::ImGui_ImplGL3_HandleMousePosition(sender->GetMouseAbsolute().x, sender->GetMouseAbsolute().y);
            sender->HandledPositionUpdate();
        }
        auto handled = imguiImpl::ImGui_ImplGL3_HandleMouse(buttonAction, mouseWheelDelta);

        if (!handled && IsRunning() && !IsPaused()) handled = HandleMouseApp(buttonAction, mouseWheelDelta, sender);
        // if (handledM)
        if (!handled && !handledMovement && IsRunning() && !IsPaused()) handled = cameraView->HandleMouse(buttonAction, mouseWheelDelta, sender);
        return handled;
    }

    /**
     * Handles resize events.
     */
    void ApplicationBase::OnResize(unsigned int width, unsigned int height)
    {
        glm::uvec2 screenSize(width, height);
        imguiImpl::ImGui_ImplGL3_Resize(static_cast<float>(width), static_cast<float>(height));
        if (orthoView) {
            orthoView->Resize(screenSize);
            orthoView->SetView();
        }
        if (cameraView) {
            cameraView->Resize(screenSize);
        }
        Resize(screenSize);
    }

    void ApplicationBase::Resize(const glm::uvec2&)
    {
    }

    void ApplicationBase::StartRun()
    {
        GLBatchRenderTarget& brt = win;
        brt.EnableAlphaBlending();
        this->m_stopped = false;
        this->m_pause = false;
        LARGE_INTEGER qwTicksPerSec = { 0, 0 };
        QueryPerformanceFrequency(&qwTicksPerSec);
        this->m_QPFTicksPerSec = qwTicksPerSec.QuadPart;

        LARGE_INTEGER qwTime;
        QueryPerformanceCounter(&qwTime);
        this->m_baseTime = qwTime.QuadPart;
        this->m_lastElapsedTime = qwTime.QuadPart;
        orthoView->SetView();
    }

    bool ApplicationBase::IsRunning() const
    {
        return !this->m_stopped;
    }

    void ApplicationBase::EndRun()
    {
        this->m_stopped = true;
    }

    void ApplicationBase::Step()
    {
        if (this->m_stopped) {
            Sleep(500);
            return;
        }

        LARGE_INTEGER qwTime;
        QueryPerformanceCounter(&qwTime);

        this->m_elapsedTime = static_cast<float>(static_cast<double>(qwTime.QuadPart - this->m_lastElapsedTime)
            / static_cast<double>(this->m_QPFTicksPerSec));
        if (this->m_elapsedTime < 0.0f)
            this->m_elapsedTime = 0.0f;

        this->m_lastElapsedTime = qwTime.QuadPart;

        if (!this->m_pause) {
            this->m_time = (qwTime.QuadPart - this->m_baseTime) / static_cast<double>(this->m_QPFTicksPerSec);

            this->FrameMove(static_cast<float>(this->m_time), static_cast<float>(this->m_elapsedTime));
            this->RenderScene();
        }

        imguiImpl::ImGui_ImplGL3_NewFrame(m_time);
        win.BatchDraw([&](GLBatchRenderTarget & rt) {
            this->RenderGUI();
            ImGui::Render();
        });
        this->win.Present();
    }
}
