/**
 * @file   ShadowMap.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.01.08
 *
 * @brief  Implementation of the shadow map class.
 */

#include "ShadowMap.h"
#include "app/ApplicationBase.h"
#include "gfx/glrenderer/GLRenderTarget.h"
#include "SpotLight.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "gfx/glrenderer/GLTexture.h"

namespace cgu {

    /**
     *  Constructor.
     *  @param size the size of the shadow map.
     *  @param app the application base object.
     */
    ShadowMap::ShadowMap(const glm::uvec2& size, const SpotLight& light, ApplicationBase* app) :
        spotLight(light),
        shadowMapSize(size),
        shadowMapRT(nullptr),
        blurredShadowMap(nullptr),
        smProgram(app->GetGPUProgramManager()->GetResource("shader/shadowMap.vp|shader/shadowMap.fp")),
        filterProgram(app->GetGPUProgramManager()->GetResource("shader/gaussianFilter.cp,TEX_FORMAT rg32f")),
        filterUniformIds(filterProgram->GetUniformLocations({ "sourceTex", "targetTex", "dir", "bloomWidth" }))
    {
        smProgram->BindUniformBlock(perspectiveProjectionUBBName, *app->GetUBOBindingPoints());

        FrameBufferDescriptor fbd;
        fbd.texDesc.emplace_back(TextureDescriptor{ 64, gl::GL_RG32F, gl::GL_RG, gl::GL_FLOAT });
        fbd.rbDesc.emplace_back(RenderBufferDescriptor{ gl::GL_DEPTH_COMPONENT32F });
        shadowMapRT.reset(new GLRenderTarget(size.x, size.y, fbd));

        blurredShadowMap = std::make_unique<GLTexture>(size.x, size.y, fbd.texDesc[0].texDesc_, nullptr);
    }

    ShadowMap::~ShadowMap() = default;

    void ShadowMap::RenderShadowGeometry(std::function<void(GLBatchRenderTarget&) > batch)
    {
        glm::vec4 bgValue(spotLight.GetCamera().GetFarZ(), spotLight.GetCamera().GetFarZ() * spotLight.GetCamera().GetFarZ(), 0.0f, 0.0f);
        shadowMapRT->GetTextures()[0]->SampleWrapBorderColor(bgValue);
        shadowMapRT->BatchDraw([&](cgu::GLBatchRenderTarget & brt) {
            brt.Clear(static_cast<unsigned int>(cgu::ClearFlags::CF_RenderTarget) | static_cast<unsigned int>(cgu::ClearFlags::CF_Depth), glm::value_ptr(bgValue), 1.0, 0);
            spotLight.GetCamera().SetViewShadowMap();
            batch(brt);
        });

        const glm::vec2 groupSize{ 32.0f, 16.0f };
        auto numGroups = glm::ivec2(glm::ceil(glm::vec2(shadowMapSize) / groupSize));

        filterProgram->UseProgram();
        filterProgram->SetUniform(filterUniformIds[0], 0);
        filterProgram->SetUniform(filterUniformIds[1], 0);
        filterProgram->SetUniform(filterUniformIds[2], glm::vec2(1.0f, 0.0f));
        filterProgram->SetUniform(filterUniformIds[3], 3.5f);
        shadowMapRT->GetTextures()[0]->ActivateTexture(gl::GL_TEXTURE0);
        blurredShadowMap->ActivateImage(0, 0, gl::GL_WRITE_ONLY);
        OGL_CALL(gl::glDispatchCompute, numGroups.x, numGroups.y, 1);
        OGL_CALL(gl::glMemoryBarrier, gl::GL_ALL_BARRIER_BITS);
        OGL_SCALL(gl::glFinish);

        filterProgram->SetUniform(filterUniformIds[2], glm::vec2(0.0f, 1.0f));
        blurredShadowMap->ActivateTexture(gl::GL_TEXTURE0);
        shadowMapRT->GetTextures()[0]->ActivateImage(0, 0, gl::GL_WRITE_ONLY);
        OGL_CALL(gl::glDispatchCompute, numGroups.x, numGroups.y, 1);
        OGL_CALL(gl::glMemoryBarrier, gl::GL_ALL_BARRIER_BITS);
        OGL_SCALL(gl::glFinish);
    }

    void ShadowMap::Resize(const glm::uvec2& smSize)
    {
        shadowMapSize = smSize;
        shadowMapRT->Resize(shadowMapSize.x, shadowMapSize.y);
        TextureDescriptor texDesc{ 64, gl::GL_RG32F, gl::GL_RG, gl::GL_FLOAT };
        blurredShadowMap = std::make_unique<GLTexture>(smSize.x, smSize.y, texDesc, nullptr);
    }

    glm::mat4 ShadowMap::GetViewProjectionTextureMatrix(const glm::mat4& view, const glm::mat4& projection)
    {
        auto translate = glm::translate(glm::mat4(), glm::vec3(0.5f, 0.5f, 0.5f));
        auto bias = glm::scale(translate, glm::vec3(0.5f, 0.5f, 0.5f));
        return bias * projection * view;
    }

    const GLTexture* ShadowMap::GetShadowTexture() const
    {
        return shadowMapRT->GetTextures()[0].get();
    }
}
