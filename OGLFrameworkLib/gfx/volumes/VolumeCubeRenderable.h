/**
 * @file   VolumeCubeRenderable.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.08.19
 *
 * @brief  Definition of a renderable used to determine starting and end positions for raycasting.
 */

#ifndef VOLUMECUBERENDERABLE_H
#define VOLUMECUBERENDERABLE_H

#include "main.h"
#include <gfx/glrenderer/ShaderMeshAttributes.h>

namespace cgu {

    class ApplicationBase;
    class GLVertexAttributeArray;
    class GPUProgram;

    struct VolumeCubeVertex
    {
        glm::vec4 pos;
        glm::vec3 posTex;
    };

    class VolumeCubeRenderable
    {
    public:
        VolumeCubeRenderable(std::shared_ptr<GPUProgram> drawProg, ApplicationBase* app);
        VolumeCubeRenderable(const VolumeCubeRenderable& orig);
        VolumeCubeRenderable(VolumeCubeRenderable&& orig);
        VolumeCubeRenderable& operator=(const VolumeCubeRenderable& orig);
        VolumeCubeRenderable& operator=(VolumeCubeRenderable&& orig);
        ~VolumeCubeRenderable();

        void Draw(float stepSize, float mipLevel = 0.0f) const;
        void DrawBack() const;

    protected:
        void Draw(GPUProgram* program, const ShaderMeshAttributes& attribBinds) const;
        void FillVertexAttributeBindings(GPUProgram* program, ShaderMeshAttributes& attribBinds) const;

    private:
        /** Holds the vertex buffer object to use. */
        BufferRAII vBuffer;
        /** Holds the index buffer object name. */
        BufferRAII iBuffer;
        /** Holds the rendering GPU program for back faces. */
        std::shared_ptr<GPUProgram> backProgram;
        /** Holds the shader attribute bindings for the back faces shader. */
        ShaderMeshAttributes backAttribBinds;
        /** Holds the rendering GPU program for drawing. */
        std::shared_ptr<GPUProgram> drawProgram;
        /** Holds the shader attribute bindings for the draw shader. */
        ShaderMeshAttributes drawAttribBinds;
        /** Holds the application object. */
        ApplicationBase* application;

        void CreateVertexIndexBuffers();
    };
}

#endif // VOLUMECUBERENDERABLE_H
