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
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>
#include <GLFW/glfw3.h>
#include <gfx/mesh/SimpleMeshRenderer.h>

namespace cgu {


    ApplicationBase::GLFWInitObject::GLFWInitObject()
    {
        glfwInit();
    }

    ApplicationBase::GLFWInitObject::~GLFWInitObject()
    {
        glfwTerminate();
    }

    /**
     * Construct a new application.
     * @param window the applications main window
     */
    ApplicationBase::ApplicationBase(const std::string& mainWindowTitle, Configuration& config, const glm::vec3& camPos) :
        pause_(true),
        stopped_(false),
        currentTime_(0.0),
        elapsedTime_(0.0),
        currentScene_(0),
        mainWin(mainWindowTitle, config),
        texManager_(),
        matManager_(),
        shaderManager_(),
        programManager_(),
        fontManager_(),
        uniformBindingPoints_(),
        shaderStorageBindingPoints_(),
        orthoView_(),
        cameraView_(),
        fontProgram_(nullptr),
        screenQuadRenderable_(nullptr)        
    {
        texManager_.reset(new TextureManager(this));
        volManager_.reset(new VolumeManager(this));
        matManager_.reset(new MaterialLibManager(this));
        shaderManager_.reset(new ShaderManager(this));
        programManager_.reset(new GPUProgramManager(this));
        fontManager_.reset(new FontManager(this));
        mainWin.RegisterApplication(*this);
        mainWin.ShowWindow();
        glm::vec2 screenSize(mainWin.GetClientSize());
        orthoView_.reset(new OrthogonalView(screenSize, &uniformBindingPoints_));
        cameraView_.reset(new ArcballCamera(60.0f, screenSize, 1.0f, 100.0f, camPos, &uniformBindingPoints_));

        fontProgram_ = programManager_->GetResource(fontProgramID);
        fontProgram_->BindUniformBlock(orthoProjectionUBBName, uniformBindingPoints_);
        screenQuadRenderable_.reset(new ScreenQuadRenderable());
        simpleMeshes_ = std::make_unique<SimpleMeshRenderer>(this);
    }

    ApplicationBase::~ApplicationBase() = default;

    /**
     * Returns the texture manager.
     * @return  the texture manager
     */
    TextureManager* ApplicationBase::GetTextureManager() const
    {
        return texManager_.get();
    }

    /**
     * Returns the volume manager.
     * @return  the volume manager
     */
    VolumeManager* ApplicationBase::GetVolumeManager() const
    {
        return volManager_.get();
    }

    /**
     * Returns the material lib manager.
     * @return the material lib manager
     */
    MaterialLibManager* ApplicationBase::GetMaterialLibManager() const
    {
        return matManager_.get();
    }

    /**
     * Returns the shader manager.
     * @return the shader manager
     */
    ShaderManager* ApplicationBase::GetShaderManager() const
    {
        return shaderManager_.get();
    }

    /**
     * Returns the GPU program manager.
     * @return the GPU program manager
     */
    GPUProgramManager* ApplicationBase::GetGPUProgramManager() const
    {
        return programManager_.get();
    }

    /**
     * Returns the uniform buffer binding points
     * @return the ubo binding points
     */
    ShaderBufferBindingPoints* ApplicationBase::GetUBOBindingPoints()
    {
        return &uniformBindingPoints_;
    }

    /**
    * Returns the shader storage buffer object binding points
    * @return the ssbo binding points
    */
    ShaderBufferBindingPoints* ApplicationBase::GetSSBOBindingPoints()
    {
        return &shaderStorageBindingPoints_;
    }

    /**
     * Returns the current configuration.
     * @return the configuration
     */
    Configuration& ApplicationBase::GetConfig() const
    {
        return mainWin.GetConfig();
    }

    /**
     * Returns the font manager.
     * @return the font manager
     */
    FontManager* ApplicationBase::GetFontManager() const
    {
        return fontManager_.get();
    }

    /**
     * Returns the main window.
     * @return the main window
     */
    GLWindow* ApplicationBase::GetWindow()
    {
        return &mainWin;
    }

    /**
     * Returns the GPU program for font rendering.
     * @return the font rendering program
     */
    std::shared_ptr<GPUProgram> ApplicationBase::GetFontProgram() const
    {
        return fontProgram_;
    }

    /**
     *  Returns the screen quad renderable.
     *  @return the screen quad renderable.
     */
    ScreenQuadRenderable* ApplicationBase::GetScreenQuadRenderable() const
    {
        return screenQuadRenderable_.get();
    }

    ArcballCamera* ApplicationBase::GetCameraView() const
    {
        return cameraView_.get();
    }

    /**
     * Handles all keyboard input.
     * @param vkCode the key pressed
     * @param bKeyDown <code>true</code> if the key is pressed, else it is released
     * @param sender the window that send the keyboard messages
     */
    bool ApplicationBase::HandleKeyboard(int key, int scancode, int action, int mods, GLWindow* sender)
    {
        auto handled = false;
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            switch (key)
            {
            case GLFW_KEY_ESCAPE:
                mainWin.CloseWindow();
                handled = true;
                break;
            case GLFW_KEY_F9:
                programManager_->RecompileAll();
                handled = true;
                break;
            }
        }

        if (!handled && IsRunning() && !IsPaused()) handled = cameraView_->HandleKeyboard(key, scancode, action, mods, sender);

        return handled;
    }

    /**
     * Handles mouse input.
     * @param buttonAction mouse action flags, see RAWMOUSE.usButtonFlags
     * @param mouseWheelDelta the scroll wheel delta
     * @param sender the window that send the keyboard messages
     * @return whether the message was handled
     */
    bool ApplicationBase::HandleMouse(int button, int action, int mods, float mouseWheelDelta, GLWindow* sender)
    {
        auto handled = false;
        if (IsRunning() && !IsPaused()) handled = HandleMouseApp(button, action, mods, mouseWheelDelta, sender);
        if (!handled && IsRunning() && !IsPaused()) handled = cameraView_->HandleMouse(button, action, mods, mouseWheelDelta, sender);
        return handled;
    }

    /**
     * Handles resize events.
     */
    void ApplicationBase::OnResize(unsigned int width, unsigned int height)
    {
        glm::uvec2 screenSize(width, height);
        if (orthoView_) {
            orthoView_->Resize(screenSize);
            orthoView_->SetView();
        }
        if (cameraView_) {
            cameraView_->Resize(screenSize);
        }
        Resize(screenSize);
    }

    void ApplicationBase::Resize(const glm::uvec2&)
    {
    }

    void ApplicationBase::StartRun()
    {
        GLBatchRenderTarget& brt = mainWin;
        brt.EnableAlphaBlending();
        stopped_ = false;
        pause_ = false;
        currentTime_ = glfwGetTime();
        orthoView_->SetView();
    }

    bool ApplicationBase::IsRunning() const
    {
        return !stopped_ && !mainWin.IsClosing();
    }

    void ApplicationBase::EndRun()
    {
        stopped_ = true;
    }

    void ApplicationBase::Step()
    {
        if (stopped_) {
            Sleep(500);
            return;
        }

        auto currentTime = glfwGetTime();
        elapsedTime_ = currentTime - currentTime_;
        currentTime_ = currentTime;
        glfwPollEvents();

        if (!this->pause_) {
            this->FrameMove(static_cast<float>(currentTime_), static_cast<float>(elapsedTime_));
            this->RenderScene();
        }

        ImGui_ImplGlfwGL3_NewFrame();
        mainWin.BatchDraw([&](GLBatchRenderTarget & rt) {
            this->RenderGUI();
            ImGui::Render();
        });
        mainWin.Present();
    }
}
