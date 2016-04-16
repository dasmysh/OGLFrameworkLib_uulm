/**
 * @file   ScreenText.cpp
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2014.02.04
 *
 * @brief  Contains the implementation of ScreenText.
 */

#include "ScreenText.h"
#include "GLVertexAttributeArray.h"
#include "gfx/Vertices.h"
#include "Font.h"
#include "core/GPUProgramManager.h"

#include <boost/assign.hpp>

namespace cgu {

    /**
    * Constructor for text in a certain direction.
    * @param fnt the font to use
    * @param programManager the GPU program manager
    * @param txt the text to render
    * @param pos the position to render at
    * @param dir the direction the text should to
    * @param fntSize the texts font size in virtual screen pixels
    * @param fntWeight the texts font weight
    * @param fntShearing the texts fonts shearing
    */
    ScreenText::ScreenText(std::shared_ptr<const Font> fnt, std::shared_ptr<GPUProgram> fontProg, const std::string& txt,
        const glm::vec2& pos, const glm::vec2& dir, const glm::vec2& fntSize, float fntWeight,
        float fntShearing, float depth) :
        font(std::move(fnt)),
        fontWeight(fntWeight),
        fontShearing(fntShearing),
        fontSize(fntSize),
        text(txt),
        position(pos),
        direction(dir),
        color(1.0f, 1.0f, 1.0f, 1.0f),
        depthLayer(depth),
        currentBuffer(0),
        fontProgram(std::move(fontProg)),
        fontMetricsBindingLocation(nullptr),
        pixelLength(0.0f)
    {
        Initialize();
    }

    /**
    * Constructor for text in a certain direction.
    * @param fnt the font to use
    * @param programManager the GPU program manager
    * @param txt the text to render
    * @param pos the position to render at
    * @param dir the direction the text should to
    * @param fntSize the texts font size in virtual screen pixels
    * @param fntWeight the texts font weight
    * @param fntShearing the texts fonts shearing
    */
    ScreenText::ScreenText(std::shared_ptr<const Font> fnt, std::shared_ptr<GPUProgram> fontProg, const std::string& txt,
        const glm::vec2& pos, const glm::vec2& dir, float fntSize, float fntWeight,
        float fntShearing, float depth) :
        ScreenText(std::move(fnt), std::move(fontProg), txt, pos, dir, glm::vec2(fntSize, fntSize), fntWeight, fntShearing, depth)
    {
    }

    /**
     * Constructor for text from left to right.
     * @param fnt the font to use
     * @param programManager the GPU program manager
     * @param txt the text to render
     * @param pos the position to render at
     * @param fntSize the texts font size in virtual screen pixels
     * @param fntWeight the texts font weight
     * @param fntShearing the texts fonts shearing
     */
    ScreenText::ScreenText(std::shared_ptr<const Font> fnt, std::shared_ptr<GPUProgram> fontProg, const std::string& txt,
        const glm::vec2& pos, float fntSize, float fntWeight, float fntShearing, float depth) :
        ScreenText(std::move(fnt), std::move(fontProg), txt, pos, glm::vec2(1.0f, 0.0f), glm::vec2(fntSize, fntSize), fntWeight, fntShearing, depth)
    {
    }

    /** Copy constructor. */
    ScreenText::ScreenText(const ScreenText& rhs) : ScreenText(rhs.font, rhs.fontProgram, rhs.text, rhs.position,
        rhs.direction, rhs.fontSize, rhs.fontWeight, rhs.fontShearing, rhs.depthLayer)
    {
    }

    /** Copy assignment operator. */
    ScreenText& ScreenText::operator=(const ScreenText& rhs)
    {
        if (this != &rhs) {
            ScreenText tmp{ rhs };
            std::swap(*this, tmp);
        }
        return *this;
    }

    /** Default move constructor. */
    ScreenText::ScreenText(ScreenText&& rhs) :
        font(std::move(rhs.font)),
        fontWeight(std::move(rhs.fontWeight)),
        fontShearing(std::move(rhs.fontShearing)),
        fontSize(std::move(rhs.fontSize)),
        text(std::move(rhs.text)),
        position(std::move(rhs.position)),
        direction(std::move(rhs.direction)),
        color(std::move(rhs.color)),
        depthLayer(std::move(rhs.depthLayer)),
        textVBOs(std::move(rhs.textVBOs)),
        textVBOFences(std::move(rhs.textVBOFences)),
        textVBOSizes(std::move(rhs.textVBOSizes)),
        currentBuffer(std::move(rhs.currentBuffer)),
        fontProgram(std::move(rhs.fontProgram)),
        vertexAttribPos(std::move(rhs.vertexAttribPos)),
        attribBind(std::move(rhs.attribBind)),
        uniformNames(std::move(rhs.uniformNames)),
        fontMetricsBindingLocation(std::move(rhs.fontMetricsBindingLocation)),
        pixelLength(std::move(rhs.pixelLength))
    {
    }

    /** Move assignment operator. */
    ScreenText& ScreenText::operator=(ScreenText&& rhs)
    {
        if (this != &rhs) {
            this->~ScreenText();
            font = std::move(rhs.font);
            fontWeight = std::move(rhs.fontWeight);
            fontShearing = std::move(rhs.fontShearing);
            fontSize = std::move(rhs.fontSize);
            text = std::move(rhs.text);
            position = std::move(rhs.position);
            direction = std::move(rhs.direction);
            color = std::move(rhs.color);
            depthLayer = std::move(rhs.depthLayer);
            textVBOs = std::move(rhs.textVBOs);
            textVBOFences = std::move(rhs.textVBOFences);
            textVBOSizes = std::move(rhs.textVBOSizes);
            currentBuffer = std::move(rhs.currentBuffer);
            fontProgram = std::move(rhs.fontProgram);
            vertexAttribPos = std::move(rhs.vertexAttribPos);
            attribBind = std::move(rhs.attribBind);
            uniformNames = std::move(rhs.uniformNames);
            fontMetricsBindingLocation = std::move(rhs.fontMetricsBindingLocation);
            pixelLength = std::move(rhs.pixelLength);
        }
        return *this;
    }

    /** Destructor. */
    ScreenText::~ScreenText()
    {
        for (auto sync : textVBOFences) {
            OGL_CALL(gl::glDeleteSync, sync);
        }
    }

    /** Initializes the object (workaround for C++98 constructors which can not call other constructors). */
    void ScreenText::Initialize()
    {
        vertexAttribPos = fontProgram->GetAttributeLocations({ "position", "index" });
        textVBOFences.resize(NUM_DYN_BUFFERS, nullptr);
        textVBOSizes.resize(NUM_DYN_BUFFERS, 0);
        for (unsigned int i = 0; i < NUM_DYN_BUFFERS; ++i) {
            attribBind.push_back(fontProgram->CreateVertexAttributeArray(textVBOs[i], 0));
        }
        InitializeText(true);
        uniformNames = fontProgram->GetUniformLocations(
            boost::assign::list_of<std::string>("fontStyle")("fontPos")("color")("fontTex"));
        fontMetricsBindingLocation = fontProgram->GetUniformBufferLocation(fontMetricsUBBName);
    }

    /**
     * Initializes the texts VBO.
     * @param first is this method called from destructor? is so, no need to choose a different buffer
     */
    void ScreenText::InitializeText(bool first)
    {
        if (!first) {
            currentBuffer = (currentBuffer + 1) % NUM_DYN_BUFFERS;
        }
        std::vector<FontVertex> textVertices(text.size());
        pixelLength = 0.0f;
        for (unsigned int i = 0; i < text.size(); ++i) {
            textVertices[i].idx[0] = font->GetCharacterId(text[i]);
            textVertices[i].pos = glm::vec3(pixelLength * direction, depthLayer);
            pixelLength += font->GetFontMetrics().chars[textVertices[i].idx[0]].xadv * fontSize.x;
        }

        if (textVBOFences[currentBuffer] != nullptr) {
            auto result = OGL_CALL(gl::glClientWaitSync, textVBOFences[currentBuffer], gl::GL_NONE_BIT, ASYNC_TIMEOUT);
            if (result == gl::GL_TIMEOUT_EXPIRED || result == gl::GL_WAIT_FAILED) {
                LOG(ERROR) << L"Waiting for buffer failed.";
                throw std::runtime_error("Waiting for buffer failed.");
            }
#ifdef _DEBUG
            if (result == gl::GL_CONDITION_SATISFIED) {
                LOG(DEBUG) << L"Waited for buffer ...";
            }
#endif
            OGL_CALL(glDeleteSync, textVBOFences[currentBuffer]);
            textVBOFences[currentBuffer] = nullptr;
        }

        OGL_CALL(gl::glBindBuffer, gl::GL_ARRAY_BUFFER, textVBOs[currentBuffer]);
        if (textVBOSizes[currentBuffer] < text.size()) {
            OGL_CALL(gl::glBufferData, gl::GL_ARRAY_BUFFER, sizeof(FontVertex) * text.size(),
                nullptr, gl::GL_DYNAMIC_DRAW);
            textVBOSizes[currentBuffer] = static_cast<gl::GLuint>(text.size());

            // TODO: Visual studio does now know EBCO. [2/23/2016 Sebastian Maisch]
            attribBind[currentBuffer]->StartAttributeSetup();
            if (vertexAttribPos[0]->iBinding >= 0) {
                attribBind[currentBuffer]->AddVertexAttribute(vertexAttribPos[0], 3,
                    gl::GL_FLOAT, gl::GL_FALSE, sizeof(FontVertex), offsetof(FontVertex, pos));
            }
            if (vertexAttribPos[1]->iBinding >= 0) {
                attribBind[currentBuffer]->AddVertexAttributeI(vertexAttribPos[1], 1,
                    gl::GL_UNSIGNED_INT, sizeof(FontVertex), offsetof(FontVertex, idx[0]));
            }
            attribBind[currentBuffer]->EndAttributeSetup();
        }

        auto ptr = OGL_CALL(gl::glMapBufferRange, gl::GL_ARRAY_BUFFER, 0, sizeof(FontVertex) * text.size(),
            gl::GL_MAP_WRITE_BIT | gl::GL_MAP_UNSYNCHRONIZED_BIT);
        if (ptr == nullptr) {
            throw std::runtime_error("Could not map text vertex buffer.");
        }

        auto buffer = static_cast<FontVertex*> (ptr);
        std::copy(textVertices.begin(), textVertices.end(), buffer);
        OGL_CALL(gl::glUnmapBuffer, gl::GL_ARRAY_BUFFER);
        OGL_CALL(gl::glBindBuffer, gl::GL_ARRAY_BUFFER, 0);
    }

    /**
     * Returns the length of the text in virtual screen pixels.
     * @return the length of the text in virtual screen pixels
     */
    float ScreenText::GetPixelLength() const
    {
        return pixelLength;
    }

    /**
     * Returns the texts baseline
     * @return the baseline
     */
    float ScreenText::GetBaseLine() const
    {
        return font->GetFontMetrics().baseLine * fontSize.y;
    }

    /**
     * Draws the text.
     * Used for drawing only a single text, so this will set the GPU programs and bind uniform blocks.
     */
    void ScreenText::Draw()
    {
        font->UseFont(fontProgram.get(), fontMetricsBindingLocation);
        DrawMultiple();
    }

    /**
     * Draws the text.
     * Used for drawing multiple texts on the screen. GPU programs and UBOs will not be set.
     */
    void ScreenText::DrawMultiple()
    {
        if (textVBOFences[currentBuffer] != nullptr) {
            OGL_CALL(glDeleteSync, textVBOFences[currentBuffer]);
        }

        OGL_CALL(gl::glBindBuffer, gl::GL_ARRAY_BUFFER, textVBOs[currentBuffer]);
        attribBind[currentBuffer]->EnableVertexAttributeArray();

        glm::vec4 fontStyle(fontWeight, fontShearing * fontSize.y * font->GetFontMetrics().sizeNormalization,
            fontSize.x * font->GetFontMetrics().sizeNormalization,
            fontSize.y * font->GetFontMetrics().sizeNormalization);
        glm::vec4 fontPos(position.x, position.y, direction.x, direction.y);
        fontProgram->SetUniform(uniformNames[0], fontStyle);
        fontProgram->SetUniform(uniformNames[1], fontPos);
        fontProgram->SetUniform(uniformNames[2], color);
        fontProgram->SetUniform(uniformNames[3], 0);
        OGL_CALL(gl::glDrawArrays, gl::GL_POINTS, 0, static_cast<gl::GLsizei>(text.size()));

        attribBind[currentBuffer]->DisableVertexAttributeArray();
        OGL_CALL(gl::glBindBuffer, gl::GL_ARRAY_BUFFER, 0);
        textVBOFences[currentBuffer] = OGL_CALL(gl::glFenceSync, gl::GL_SYNC_GPU_COMMANDS_COMPLETE, gl::GL_NONE_BIT);
    }

    /**
     * Sets a new text to this object.
     * @param txt the text to set
     * @param reinit reinitialize the vertex buffer (use <code>false</code> if you want to make multiple
     * changes and update the buffer on the last change)
     */
    void ScreenText::SetText(const std::string& txt, bool reinit)
    {
        text = txt;
        if (reinit) InitializeText();
    }

    /**
     * Sets the texts position on the virtual screen.
     * The y position will be the texts top left corner.
     * @param pos the new position of the text
     */
    void ScreenText::SetPosition(const glm::vec2& pos)
    {
        position = pos;
    }

    /**
     * Sets the direction in which the text should go
     * @param dir the new direction
     * @param reinit reinitialize the vertex buffer (use <code>false</code> if you want to make multiple
     * changes and update the buffer on the last change)
     */
    void ScreenText::SetDirection(const glm::vec2& dir, bool reinit)
    {
        direction = dir;
        if (reinit) InitializeText();
    }

    /**
     * Sets the text size in virtual screen pixels. Scales the text uniform.
     * @param size the new text size
     * @param reinit reinitialize the vertex buffer (use <code>false</code> if you want to make multiple
     * changes and update the buffer on the last change)
     */
    void ScreenText::SetFontSize(float size, bool reinit)
    {
        fontSize = glm::vec2(size, size);
        if (reinit) InitializeText();
    }

    /**
     * Sets the texts size in virtual screen pixels. Scales the text non-uniform.
     * @param size the new non-uniform text size
     * @param reinit reinitialize the vertex buffer (use <code>false</code> if you want to make multiple
     * changes and update the buffer on the last change)
     */
    void ScreenText::SetFontSize(const glm::vec2& size, bool reinit)
    {
        fontSize = size;
        if (reinit) InitializeText();
    }

    /**
     * Sets the depth layer the text should be shown on. This is used for the depth buffer test.
     * @param depth the new depth. This value should vary between -1.0f and 1.0f
     * @param reinit reinitialize the vertex buffer (use <code>false</code> if you want to make multiple
     * changes and update the buffer on the last change)
     */
    void ScreenText::SetDepthLayer(float depth, bool reinit)
    {
        depthLayer = depth;
        if (reinit) InitializeText();
    }

    /**
     * Sets the fonts weight.
     * @param weight the new font weight.
     */
    void ScreenText::SetFontWeight(float weight)
    {
        fontWeight = weight;
    }

    /**
     * Sets the fonts shearing (or italic-ness).
     * @param shearing describes how much the fonts top is shifted towards direction in percent according to font height
     * (1 = 100% means the text will be shifted by exactly the fonts height).
     */
    void ScreenText::SetFontShearing(float shearing)
    {
        fontShearing = shearing;
    }

    /**
     * Sets the fonts color.
     * @param col the new color
     */
    void ScreenText::SetColor(const glm::vec4& col)
    {
        color = col;
    }
}
