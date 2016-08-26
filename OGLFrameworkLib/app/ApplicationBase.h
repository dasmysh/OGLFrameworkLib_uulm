/**
 * @file   ApplicationBase.h
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2013.12.17
 *
 * @brief  Declares the application base class.
 */

#ifndef APPLICATIONBASE_H
#define APPLICATIONBASE_H

#include "core/TextureManager.h"
#include "gfx/glrenderer/ShaderBufferBindingPoints.h"
#include "core/MaterialLibManager.h"
#include "core/ShaderManager.h"
#include "core/GPUProgramManager.h"
#include "core/FontManager.h"
#include "gfx/OrthogonalView.h"
#include "gfx/ArcballCamera.h"
#include "main.h"
#include "core/VolumeManager.h"
#include "gfx/glrenderer/ScreenQuadRenderable.h"
#include "GLWindow.h"

namespace cgu {

    class SimpleMeshRenderer;

    /**
     * @brief Application base.
     * <para>    Base class for all applications using this framework. </para>
     *
     * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
     * @date   2013.12.17
     */
    class ApplicationBase
    {
    public:
        ApplicationBase(const std::string& mainWindowTitle, Configuration& config, const glm::vec3& camPos);
        ApplicationBase(const ApplicationBase&) = delete;
        ApplicationBase& operator=(const ApplicationBase&) = delete;
        virtual ~ApplicationBase();

        /** Starts the application. */
        void StartRun();
        /** Checks if the application is still running. */
        bool IsRunning() const;
        /** Make one application <em>step</em> (rendering etc.). */
        void Step();
        /** Called if the application is to end running. */
        void EndRun();

        bool IsPaused() const { return pause_; }
        void SetPause(bool pause) { pause_ = pause; }

        virtual bool HandleKeyboard(int key, int scancode, int action, int mods, GLWindow* sender);
        bool HandleMouse(int button, int action, int mods, float mouseWheelDelta, GLWindow* sender);
        virtual bool HandleMouseApp(int button, int action, int mods, float mouseWheelDelta, GLWindow* sender) = 0;
        void OnResize(unsigned int width, unsigned int height);
        virtual void Resize(const glm::uvec2& screenSize);

        TextureManager* GetTextureManager() const;
        VolumeManager* GetVolumeManager() const;
        MaterialLibManager* GetMaterialLibManager() const;
        ShaderManager* GetShaderManager() const;
        GPUProgramManager* GetGPUProgramManager() const;
        FontManager* GetFontManager() const;
        ShaderBufferBindingPoints* GetUBOBindingPoints();
        ShaderBufferBindingPoints* GetSSBOBindingPoints();
        Configuration& GetConfig() const;
        GLWindow* GetWindow();
        std::shared_ptr<GPUProgram> GetFontProgram() const;
        ScreenQuadRenderable* GetScreenQuadRenderable() const;
        ArcballCamera* GetCameraView() const;
        OrthogonalView* GetOrthoginalView() const { return orthoView_.get(); }
        const SimpleMeshRenderer* GetSimpleMeshes() const { return simpleMeshes_.get(); }

    private:
        class GLFWInitObject
        {
        public:
            GLFWInitObject();
            ~GLFWInitObject();
        };

        GLFWInitObject forceGLFWInit;

        // application status
        /** <c>true</c> if application is paused. */
        bool pause_;
        /**  <c>true</c> if the application has stopped (i.e. the last scene has finished). */
        bool stopped_;
        /** The (global) time of the application. */
        double currentTime_;
        /** Time elapsed in the frame. */
        double elapsedTime_;
        /** The current scene. */
        unsigned int currentScene_;

    protected:
        /**
         * Moves a single frame.
         * @param time the time elapsed since the application started
         * @param elapsed the time elapsed since the last frame
         */
        virtual void FrameMove(float time, float elapsed) = 0;
        /** Render the scene. */
        virtual void RenderScene() = 0;
        /** Render the scenes GUI. */
        virtual void RenderGUI() = 0;

    private:
        /** Holds the applications main window. */
        GLWindow mainWin;
        /** Holds the texture manager. */
        std::unique_ptr<TextureManager> texManager_;
        /** Holds the volume manager. */
        std::unique_ptr<VolumeManager> volManager_;
        /** Holds the material lib manager. */
        std::unique_ptr<MaterialLibManager> matManager_;
        /** Holds the shader manager. */
        std::unique_ptr<ShaderManager> shaderManager_;
        /** Holds the GPU program manager. */
        std::unique_ptr<GPUProgramManager> programManager_;
        /** Holds the font manager. */
        std::unique_ptr<FontManager> fontManager_;

        /** Holds the uniform binding points. */
        ShaderBufferBindingPoints uniformBindingPoints_;
        /** Holds the shader storage buffer object binding points. */
        ShaderBufferBindingPoints shaderStorageBindingPoints_;
        /** Holds the orthographic view. */
        std::unique_ptr<OrthogonalView> orthoView_;
        /** Holds the perspective camera view. */
        std::unique_ptr<ArcballCamera> cameraView_;
        /** Holds the GPUProgram for font rendering. */
        std::shared_ptr<GPUProgram> fontProgram_;
        /** Holds the screen quad renderable. */
        std::unique_ptr<ScreenQuadRenderable> screenQuadRenderable_;
        /** Holds the simple meshes renderer. */
        std::unique_ptr<SimpleMeshRenderer> simpleMeshes_;
    };
}
#endif /* APPLICATIONBASE_H */
