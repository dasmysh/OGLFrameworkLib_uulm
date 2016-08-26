/**
 * @file   SimpleMeshRenderer.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.08.26
 *
 * @brief  Declaration of the SimpleMeshRenderer class.
 */

#ifndef SIMPLEMESHRENDERER_H
#define SIMPLEMESHRENDERER_H

#include "main.h"
#include <gfx/glrenderer/ShaderMeshAttributes.h>

namespace cgu {

    class GPUProgram;
    class GLBuffer;

    class SimpleMeshRenderer
    {
    public:
        explicit SimpleMeshRenderer(ApplicationBase* app);
        ~SimpleMeshRenderer();

        void DrawCone(const glm::mat4& modelMatrix, const glm::vec4& color) const;
        void DrawCube(const glm::mat4& modelMatrix, const glm::vec4& color) const;
        void DrawCylinder(const glm::mat4& modelMatrix, const glm::vec4& color) const;
        void DrawOctahedron(const glm::mat4& modelMatrix, const glm::vec4& color) const;
        void DrawSphere(const glm::mat4& modelMatrix, const glm::vec4& color) const;
        void DrawTorus(const glm::mat4& modelMatrix, const glm::vec4& color) const;
        void DrawPoint(const glm::mat4& modelMatrix, const glm::vec4& color, float pointSize) const;
        void DrawLine(const glm::mat4& modelMatrix, const glm::vec4& color) const;

    private:
        void DrawSubmesh(const glm::mat4& modelMatrix, const glm::vec4& color, unsigned int submeshId, float pointSize = 1.0f) const;

        using SimpleSubMesh = std::pair<unsigned int, unsigned int>;

        /** Holds the sub mesh information. */
        std::array<SimpleSubMesh, 8> submeshInfo_;
        /** Holds the simple GPU program for mesh rendering. */
        std::shared_ptr<GPUProgram> simpleProgram_;
        /** Holds the vertex buffer. */
        std::unique_ptr<GLBuffer> vBuffer_;
        /** Holds the index buffer of the mesh base. */
        std::unique_ptr<GLBuffer> iBuffer_;
        /** Holds the shader attribute bindings for the shader. */
        ShaderMeshAttributes drawAttribBinds_;
    };
}

#endif // SIMPLEMESHRENDERER_H
