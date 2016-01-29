/**
 * @file   TransferFunctionGUI.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.08.24
 *
 * @brief  Declaration of the transfer function GUI class.
 */

#ifndef TRANSFERFUNCTIONGUI_H
#define TRANSFERFUNCTIONGUI_H

#include "main.h"
#include "TransferFunction.h"
#include "gfx/glrenderer/GLVertexAttributeArray.h"
#include "gfx/OrthogonalView.h"

namespace cgu {

    class BaseGLWindow;
    class ScreenQuadRenderable;
    class GLTexture;
    class GPUProgram;

    class TransferFunctionGUI
    {
    public:
        TransferFunctionGUI(const glm::vec2& boxMin, const glm::vec2& boxMax, ApplicationBase* app);
        ~TransferFunctionGUI();

        void Draw();
        void Resize(const glm::uvec2& screenSize);
        bool HandleMouse(unsigned int buttonAction, float mouseWheelDelta, BaseGLWindow* sender);
        void LoadTransferFunctionFromFile(const std::string& filename);
        void InitTF(float freq);
        const GLTexture* GetTexture() const { return tfTex.get(); };

    private:
        static const int PADDING = 10;
        static const int CP_GUI_WIDTH = 128;
        static const int CP_GUI_HEIGHT = 128;
        static const int TEX_RES = 512;
        const float pickRadius = 10.0f;

        bool SelectPoint(const glm::vec2& position, const glm::vec2& pickSize);
        bool AddPoint(const glm::vec2& position, const glm::vec2& pickSize);
        bool RemovePoint(const glm::vec2& position, const glm::vec2& pickSize);
        void UpdateTF(bool createVAO = false);

        static bool Overlap(const glm::vec2& point)
        {
            if (point.x < 0.0f || point.x > 1.0f) return false;
            if (point.y < 0.0f || point.y > 1.0f) return false;
            return true;
        }

        /** Holds the minimum and maximum position of the rectangle. */
        glm::vec2 rectMin; glm::vec2 rectMax;
        /** Holds the ortho projection buffer. */
        OrthoProjectionBuffer orthoBuffer;

        /** holds the GUI renderable for a textured quad. */
        std::unique_ptr<ScreenQuadRenderable> quad;
        /** holds the quads texture. */
        std::unique_ptr<GLTexture> quadTex;
        /** The texture that is the result of the transfer function. */
        std::unique_ptr<GLTexture> tfTex;
        /** holds the GPU program for rendering screen aligned things. */
        std::shared_ptr<GPUProgram> screenAlignedProg;
        /** holds the uniform binding point for textures on screen aligned things. */
        BindingLocation screenAlignedTextureUniform;
        /** holds the index of the selected point (-1 for no selection). */
        int selection;
        /** holds whether the selection is currently dragged. */
        bool draggingSelection;
        /** Holds the last mouse button action. */
        unsigned int lastButtonAction;

        /** Holds the shader for rendering the transfer function. */
        std::shared_ptr<GPUProgram> tfProgram;
        /** Holds the orthographic projection uniform buffer. */
        std::unique_ptr<GLUniformBuffer> orthoUBO;
        /** holds the VBO for the transfer function. */
        BufferRAII tfVBO;
        /** Holds the vertex attribute bindings for the shader. */
        GLVertexAttributeArray* attribBind;

        /** Holds the filename to save the transfer function to. */
        std::string saveTFFilename;

        void UpdateTexture() const;
        int GetControlPoint(const glm::vec2& p);
        tf::TransferFunction tf_;
    };
}

#endif // TRANSFERFUNCTIONGUI_H
