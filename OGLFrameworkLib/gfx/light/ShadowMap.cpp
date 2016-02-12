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
#include <glm/gtc/matrix_transform.inl>

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
        smProgram(app->GetGPUProgramManager()->GetResource("shader/shadowMap.vp|shader/shadowMap.fp"))
    {
        smProgram->BindUniformBlock(perspectiveProjectionUBBName, *app->GetUBOBindingPoints());

        FrameBufferDescriptor fbd;
        fbd.texDesc.emplace_back(4, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT);
        shadowMapRT.reset(new GLRenderTarget(size.x, size.y, fbd));
        shadowMapRT->GetTextures()[0]->ActivateShadowMapComparison();
    }

    ShadowMap::~ShadowMap() = default;

    void ShadowMap::RenderShadowGeometry(std::function<void(const CameraView&, GLBatchRenderTarget&) > batch)
    {
        LOG(WARNING) << "SMRT: " << shadowMapRT->GetTextures()[0]->GetGLIdentifier().textureId;
        shadowMapRT->BatchDraw([&](cgu::GLBatchRenderTarget & brt) {
            brt.Clear(static_cast<unsigned int>(cgu::ClearFlags::CF_Depth), nullptr, 1.0, 0);

            batch(spotLight.GetCamera(), brt);
        });
    }

    void ShadowMap::Resize(const glm::uvec2& smSize)
    {
        shadowMapSize = smSize;
        shadowMapRT->Resize(shadowMapSize.x, shadowMapSize.y);
        shadowMapRT->GetTextures()[0]->ActivateShadowMapComparison();
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
