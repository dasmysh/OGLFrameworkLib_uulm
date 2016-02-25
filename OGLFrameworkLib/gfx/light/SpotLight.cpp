/**
 * @file   SpotLight.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.12.17
 *
 * @brief  Implementation of the spotlight class.
 */

#include "SpotLight.h"
#include "gfx/glrenderer/GLUniformBuffer.h"
#include "ShadowMap.h"
#include "app/ApplicationBase.h"

namespace cgu {

    /**
     *  Constructor.
     *  @param intensity the lights power.
     *  @param theFov the light spots angle.
     *  @param pos the lights position.
     *  @param smSize size of the shadow map.
     *  @param uniformBindingPoints uniform buffer binding points for the camera used for shadow map rendering.
     */
    SpotLight::SpotLight(const glm::vec3&  intensity, float theFov, const glm::vec3& pos, const glm::uvec2& smSize, ApplicationBase* app) :
        camera(RI_MOUSE_RIGHT_BUTTON_DOWN, MB_RGHT, theFov, smSize, 0.1f, 100.0f, pos, app->GetUBOBindingPoints()),
        falloffWidth(0.05f),
        intensity(intensity),
        attenuation(1.0f / 128.0f),
        farPlane(10.0f),
        bias(-0.01f),
        shadowMap(new ShadowMap(smSize, *this, app)),
        application(app)
    {
    }

    /** Copy constructor. */
    SpotLight::SpotLight(const SpotLight& rhs) :
        SpotLight(rhs.intensity, rhs.GetCamera().GetFOV(), rhs.GetCamera().GetPosition(), rhs.GetShadowMap()->GetSize(), rhs.application)
    {
    }

    /** Copy assignment operator. */
    SpotLight& SpotLight::operator=(const SpotLight& rhs)
    {
        if (this != &rhs) {
            SpotLight tmp{ rhs };
            std::swap(*this, tmp);
        }
        return *this;
    }

    /** Move constructor. */
    SpotLight::SpotLight(SpotLight&& rhs) :
        camera(std::move(rhs.camera)),
        falloffWidth(rhs.falloffWidth),
        intensity(rhs.intensity),
        attenuation(rhs.attenuation),
        farPlane(rhs.farPlane),
        bias(rhs.bias),
        shadowMap(std::move(rhs.shadowMap)),
        application(rhs.application)
    {

    }

    /** Move assignment operator. */
    SpotLight& SpotLight::operator=(SpotLight&& rhs)
    {
        if (this != &rhs) {
            camera = std::move(rhs.camera);
            falloffWidth = rhs.falloffWidth;
            intensity = rhs.intensity;
            attenuation = rhs.attenuation;
            farPlane = rhs.farPlane;
            bias = rhs.bias;
            shadowMap = std::move(rhs.shadowMap);
            application = rhs.application;
        }
        return *this;
    }

    /** Default destructor. */
    SpotLight::~SpotLight() = default;

    void SpotLight::Resize(const glm::uvec2& screenSize)
    {
        camera.Resize(screenSize);
        shadowMap->Resize(screenSize);
    }

    /**
     *  Handles keyboard input for light positioning.
     *  @param vkCode the virtual key code of the key event.
     *  @param vKeyDown whether the key is down or not.
     *  @param sender the window the event came from.
     */
    bool SpotLight::HandleKeyboard(unsigned int vkCode, bool bKeyDown, BaseGLWindow* sender)
    {
        return camera.HandleKeyboard(vkCode, bKeyDown, sender);
    }

    /**
     *  Handles the mouse input for camera positioning.
     *  @param buttonAction the button event.
     *  @param mouseWheelDelta the mouse wheel movement.
     *  @param sender the window the event came from.
     */
    bool SpotLight::HandleMouse(unsigned int buttonAction, float mouseWheelDelta, BaseGLWindow* sender)
    {
        return camera.HandleMouse(buttonAction, mouseWheelDelta, sender);
    }

    /**
     *  Updates the light position and direction.
     */
    void SpotLight::UpdateLight()
    {
        camera.UpdateCamera();
    }

    /**
     *  Updates the lights parameters.
     *  @param params the light parameters to update.
     *  @param nextTextureUnit the texture unit to use for this lights shadow map.
     *  @return the next texture unit.
     */
    int SpotLight::UpdateLightParameters(SpotLightParams& params, int nextTextureUnit) const
    {
        params.position = glm::vec4(camera.GetPosition(), 1.0f);
        params.direction = glm::vec4(glm::normalize(glm::vec3(0.0f) - camera.GetPosition()), 1.0f);
        params.intensity = glm::vec4(intensity, 1.0f);
        params.angFalloffStart = glm::cos(0.5f * camera.GetFOV());
        params.angFalloffWidth = falloffWidth;
        params.distAttenuation = attenuation;
        params.farZ = farPlane;
        params.viewProjection = shadowMap->GetViewProjectionTextureMatrix(camera.GetViewMatrix(), camera.GetProjMatrix());
        shadowMap->GetShadowTexture()->ActivateTexture(GL_TEXTURE0 + nextTextureUnit);
        return nextTextureUnit + 1;
    }

    /**
     *  Constructor.
     */
    SpotLightArray::SpotLightArray(const std::string& lightArrayName, ShaderBufferBindingPoints* uniformBindingPoints) :
        lightsUBO(new GLUniformBuffer(lightArrayName, sizeof(SpotLightParams), uniformBindingPoints))
    {
    }

    /** Default destructor. */
    SpotLightArray::~SpotLightArray() = default;

    /**
     *  Sets the light array parameters.
     *  @param firstTextureUnit the first texture unit to use.
     *  @return the next free texture unit.
     */
    int SpotLightArray::SetLightParameters(int firstTextureUnit, std::vector<int>& shadowMapTextureUnits)
    {
        assert(shadowMapTextureUnits.size() == lights.size());

        if (lights.size() != lightParams.size()) {
            auto uniformBindingPoints = lightsUBO->GetBindingPoints();
            auto lightArrayName = lightsUBO->GetUBOName();
            lightParams.resize(lights.size());
            lightsUBO.reset(new GLUniformBuffer(lightArrayName, sizeof(SpotLightParams) * static_cast<unsigned int>(lightParams.size()), uniformBindingPoints));
        }

        auto nextTexture = firstTextureUnit;
        for (unsigned int i = 0; i < lights.size(); ++i) {
            shadowMapTextureUnits[i] = nextTexture;
            nextTexture = lights[i].UpdateLightParameters(lightParams[i], nextTexture);
        }

        lightsUBO->UploadData(0, sizeof(SpotLightParams) * static_cast<unsigned int>(lightParams.size()), lightParams.data());
        lightsUBO->BindBuffer();
        return nextTexture;
    }
}
