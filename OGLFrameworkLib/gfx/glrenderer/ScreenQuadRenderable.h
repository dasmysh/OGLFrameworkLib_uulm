/**
 * @file   ScreenQuadRenderable.h
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2015.21.12
 *
 * @brief  Contains the definition of ScreenQuadRenderable.
 */

#ifndef SIMPLEQUADRENDERABLE_H
#define SIMPLEQUADRENDERABLE_H

#include "main.h"

namespace cgu {

    class GPUProgram;

    /**
     * @brief
     *
     * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
     * @date   2015.21.12
     */
    class ScreenQuadRenderable
    {
    public:
        ScreenQuadRenderable();
        ScreenQuadRenderable(const std::array <glm::vec2, 4>& vertices, std::shared_ptr<GPUProgram> program);
        ScreenQuadRenderable(const ScreenQuadRenderable&);
        ScreenQuadRenderable& operator=(const ScreenQuadRenderable&);
        ScreenQuadRenderable(ScreenQuadRenderable&& orig);
        ScreenQuadRenderable& operator=(ScreenQuadRenderable&& orig);
        ~ScreenQuadRenderable();

        void Draw() const;

    private:
        /** Holds the vertex data in the buffer. */
        std::array<glm::vec2, 4> vertexData;
        /** Holds the (optional) program used for rendering. */
        std::shared_ptr<GPUProgram> program;
        /** Holds the vertex buffer object to use. */
        BufferRAII vBuffer;
        /** Holds the vertex attribute bindings. */
        std::unique_ptr<GLVertexAttributeArray> vertexAttribs;

        void FillAttributeBindings();
    };
}

#endif /* SIMPLEQUADRENDERABLE_H */
