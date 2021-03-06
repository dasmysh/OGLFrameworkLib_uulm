/**
 * @file   FilmicTMOperator.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.12.21
 *
 * @brief  Declaration of the filmic tone-mapping operator class.
 */

#ifndef FILMICTMOPERATOR_H
#define FILMICTMOPERATOR_H

#include "main.h"

namespace cgu {

    class GLRenderTarget;
    class GPUProgram;
    class GLUniformBuffer;
    class ScreenQuadRenderable;
    class GLTexture;


    struct FilmicTMParameters
    {
        float sStrength;
        float linStrength;
        float linAngle;
        float toeStrength;
        float toeNumerator;
        float toeDenominator;
        float white;
        float exposure;
    };

    /**
     *  Filmic tone-mapping operator.
     *  @see http://filmicgames.com/archives/75
     */
    class FilmicTMOperator
    {
    public:
        explicit FilmicTMOperator(ApplicationBase* app);
        ~FilmicTMOperator();

        void RenderParameterSliders();
        void ApplyTonemapping(GLTexture* sourceRT, GLRenderTarget* targetRT);
        void Resize(const glm::uvec2& screenSize);

        void SetExposure(float exposure) { params.exposure = exposure; }
        float GetExposure() const { return params.exposure; }

        void SaveParameters(std::ostream& ostr) const;
        void LoadParameters(std::istream& istr);

    private:
        static const unsigned int VERSION = 1;

        /** Holds the shader used for tone-mapping. */
        std::shared_ptr<GPUProgram> tmProgram;
        /** Holds the screen renderable for the tone-mapping. */
        cgu::ScreenQuadRenderable* renderable;
        /** Holds the shader uniform ids. */
        std::vector<BindingLocation> uniformIds;
        /** Holds the parameters for the tone-mapping. */
        FilmicTMParameters params;
        /** Holds the filmic uniform buffer. */
        std::unique_ptr<GLUniformBuffer> filmicUBO;
    };
}



#endif // FILMICTMOPERATOR_H
