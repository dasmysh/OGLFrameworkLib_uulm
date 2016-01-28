/**
 * @file  ScreenQuadRenderable.cpp
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2015.21.12
 *
 * @brief  Contains the implementation of SimpleQuadRenderable.
 */

#include "ScreenQuadRenderable.h"
#include "gfx/glrenderer/GLVertexAttributeArray.h"
#include <boost/assign.hpp>
#include "gfx/glrenderer/GPUProgram.h"

namespace cgu {

    /** Default constructor. */
    ScreenQuadRenderable::ScreenQuadRenderable() :
        ScreenQuadRenderable(std::array<glm::vec2, 4>(), nullptr)
    {
    }

    /**
     *  Constructor for externally supplied vertices.
     *  @param vertices the vertices to use.
     *  @param prog the program to use (optional).
     */
    ScreenQuadRenderable::ScreenQuadRenderable(const std::array <glm::vec2, 4>& vertices, std::shared_ptr<GPUProgram> prog) :
        vertexData(vertices),
        program(prog)
    {
        OGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, vBuffer);
        OGL_CALL(glBufferData, GL_ARRAY_BUFFER, 4 * sizeof(glm::vec2), vertices.data(), GL_STATIC_DRAW);

        FillAttributeBindings();
    }

    /** Copy constructor. */
    ScreenQuadRenderable::ScreenQuadRenderable(const ScreenQuadRenderable& rhs) :
        ScreenQuadRenderable(rhs.vertexData, rhs.program)
    {
    }

    /** Copy assignment operator. */
    ScreenQuadRenderable& ScreenQuadRenderable::operator=(const ScreenQuadRenderable& rhs)
    {
        ScreenQuadRenderable tmp{ rhs };
        std::swap(*this, tmp);
        return *this;
    }

    /** Move constructor. */
    ScreenQuadRenderable::ScreenQuadRenderable(ScreenQuadRenderable&& rhs) :
        vertexData(std::move(rhs.vertexData)),
        program(std::move(rhs.program)),
        vBuffer(std::move(rhs.vBuffer)),
        vertexAttribs(std::move(rhs.vertexAttribs))
    {
    }

    /** Move assignment operator. */
    ScreenQuadRenderable& ScreenQuadRenderable::operator=(ScreenQuadRenderable&& rhs)
    {
        if (this != &rhs) {
            this->~ScreenQuadRenderable();
            vertexData = std::move(rhs.vertexData);
            program = std::move(rhs.program);
            vBuffer = std::move(rhs.vBuffer);
            vertexAttribs = std::move(rhs.vertexAttribs);
        }
        return *this;
    }

    ScreenQuadRenderable::~ScreenQuadRenderable() = default;

    void ScreenQuadRenderable::FillAttributeBindings()
    {
        OGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, vBuffer);
        vertexAttribs.reset(new GLVertexAttributeArray(vBuffer, 0));

        vertexAttribs->StartAttributeSetup();
        if (program != nullptr) {
            auto shaderPositions = program->GetAttributeLocations({ "pos" });
            vertexAttribs->AddVertexAttribute(shaderPositions[0], 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0);
        }
        vertexAttribs->EndAttributeSetup();
        OGL_CALL(glBindBuffer, GL_ARRAY_BUFFER, 0);
    }

    void ScreenQuadRenderable::Draw() const
    {
        vertexAttribs->EnableVertexAttributeArray();
        OGL_CALL(glDrawArrays, GL_TRIANGLE_STRIP, 0, 4);
        vertexAttribs->DisableVertexAttributeArray();
    }
}
