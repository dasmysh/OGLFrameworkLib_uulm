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

namespace cgu {

    /**
     *  Constructor.
     *  @param size the size of the shadow map.
     *  @param app the application base object.
     */
    ShadowMap::ShadowMap(const glm::uvec2& size, const SpotLight& light, ApplicationBase* app) :
        spotLight(light),
        shadowMapRT(nullptr),
        smProgram(app->GetGPUProgramManager()->GetResource("shader/shadowMap.vp|shader/shadowMap.fp"))
    {
        smProgram->BindUniformBlock(perspectiveProjectionUBBName, *app->GetUBOBindingPoints());

        FrameBufferDescriptor fbd;
        fbd.texDesc.emplace_back(32, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT);
        shadowMapRT.reset(new GLRenderTarget(size.x, size.y, fbd));
    }

    ShadowMap::~ShadowMap() = default;

    void ShadowMap::RenderShadowGeometry(std::function<void(const CameraView&, GLBatchRenderTarget&) > batch)
    {
        shadowMapRT->BatchDraw([&](cgu::GLBatchRenderTarget & brt) {
            brt.Clear(static_cast<unsigned int>(cgu::ClearFlags::CF_Depth), nullptr, 1.0, 0);

            batch(spotLight.GetCamera(), brt);
        });
    }
}
