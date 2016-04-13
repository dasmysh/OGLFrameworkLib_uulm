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
#include <GLFW/glfw3.h>

namespace cgu {


    SpotLight::SpotLight(const glm::vec3& intensity, float fov, const glm::vec3& pos, std::unique_ptr<ShadowMap> shadowMap, ApplicationBase* app) :
        camera_(GLFW_MOUSE_BUTTON_RIGHT, fov, shadowMap->GetSize(), 0.1f, 100.0f, pos, app->GetUBOBindingPoints()),
        falloffWidth_(0.05f),
        intensity_(intensity),
        attenuation_(1.0f / 128.0f),
        bias_(-0.01f),
        shadowMap_(std::move(shadowMap)),
        application_(app)
    {
    }

    SpotLight::SpotLight(const glm::vec3& intensity, float fov, const glm::vec3& pos, const glm::uvec2& smSize,
        unsigned smComponents, const std::shared_ptr<GPUProgram>& smProgram, const std::shared_ptr<GPUProgram>& filterProgram, ApplicationBase* app) :
        SpotLight(intensity, fov, pos, std::make_unique<ShadowMap>(smSize, smComponents, *this, smProgram, filterProgram, app), app)
    {
    }

    /**
     *  Constructor.
     *  @param intensity the lights power.
     *  @param theFov the light spots angle.
     *  @param pos the lights position.
     *  @param smSize size of the shadow map.
     *  @param uniformBindingPoints uniform buffer binding points for the camera used for shadow map rendering.
     */
    SpotLight::SpotLight(const glm::vec3&  intensity, float theFov, const glm::vec3& pos, const glm::uvec2& smSize, ApplicationBase* app) :
        SpotLight(intensity, theFov, pos, std::make_unique<ShadowMap>(smSize, *this, app), app)
    {
    }

    /** Copy constructor. */
    SpotLight::SpotLight(const SpotLight& rhs) :
        SpotLight(rhs.intensity_, rhs.GetCamera().GetFOV(), rhs.GetCamera().GetPosition(), rhs.GetShadowMap()->GetSize(),
        rhs.GetShadowMap()->GetShadowTexture()->GetDescriptor().bytesPP / 32, rhs.GetShadowMap()->GetShadowMappingProgram(),
        rhs.GetShadowMap()->GetFilteringProgram(), rhs.application_)
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
        camera_(std::move(rhs.camera_)),
        falloffWidth_(rhs.falloffWidth_),
        intensity_(rhs.intensity_),
        attenuation_(rhs.attenuation_),
        bias_(rhs.bias_),
        shadowMap_(std::move(rhs.shadowMap_)),
        application_(rhs.application_)
    {

    }

    /** Move assignment operator. */
    SpotLight& SpotLight::operator=(SpotLight&& rhs)
    {
        if (this != &rhs) {
            camera_ = std::move(rhs.camera_);
            falloffWidth_ = rhs.falloffWidth_;
            intensity_ = rhs.intensity_;
            attenuation_ = rhs.attenuation_;
            bias_ = rhs.bias_;
            shadowMap_ = std::move(rhs.shadowMap_);
            application_ = rhs.application_;
        }
        return *this;
    }

    /** Default destructor. */
    SpotLight::~SpotLight() = default;

    void SpotLight::Resize(const glm::uvec2& screenSize)
    {
        camera_.Resize(screenSize);
        shadowMap_->Resize(screenSize);
    }

    /**
     *  Handles keyboard input for light positioning.
     *  @param vkCode the virtual key code of the key event.
     *  @param vKeyDown whether the key is down or not.
     *  @param sender the window the event came from.
     */
    bool SpotLight::HandleKeyboard(int key, int scancode, int action, int mods, GLWindow* sender)
    {
        return camera_.HandleKeyboard(key, scancode, action, mods, sender);
    }

    /**
     *  Handles the mouse input for camera positioning.
     *  @param buttonAction the button event.
     *  @param mouseWheelDelta the mouse wheel movement.
     *  @param sender the window the event came from.
     */
    bool SpotLight::HandleMouse(int button, int action, int mods, float mouseWheelDelta, GLWindow* sender)
    {
        return camera_.HandleMouse(button, action, mods, mouseWheelDelta, sender);
    }

    /**
     *  Updates the light position and direction.
     */
    void SpotLight::UpdateLight()
    {
        camera_.UpdateCamera();
    }

    /**
     *  Updates the lights parameters.
     *  @param params the light parameters to update.
     *  @param nextTextureUnit the texture unit to use for this lights shadow map.
     *  @return the next texture unit.
     */
    int SpotLight::UpdateLightParameters(SpotLightParams& params, int nextTextureUnit) const
    {
        params.position = glm::vec4(camera_.GetPosition(), 1.0f);
        params.direction = glm::vec4(glm::normalize(glm::vec3(0.0f) - camera_.GetPosition()), 1.0f);
        params.intensity = glm::vec4(intensity_, 1.0f);
        params.angFalloffStart = glm::cos(0.5f * camera_.GetFOV());
        params.angFalloffWidth = falloffWidth_;
        params.distAttenuation = attenuation_;
        params.farZ = camera_.GetFarZ();
        params.viewProjection = shadowMap_->GetViewProjectionTextureMatrix(camera_.GetViewMatrix(), camera_.GetProjMatrix());
        shadowMap_->GetShadowTexture()->ActivateTexture(GL_TEXTURE0 + nextTextureUnit);
        return nextTextureUnit + 1;
    }

    /**
     *  Constructor.
     */
    SpotLightArray::SpotLightArray(const std::string& lightArrayName, ShaderBufferBindingPoints* uniformBindingPoints) :
        lightsUBO_(new GLUniformBuffer(lightArrayName, sizeof(SpotLightParams), uniformBindingPoints))
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
        assert(shadowMapTextureUnits.size() == lights_.size());

        if (lights_.size() != lightParams_.size()) {
            auto uniformBindingPoints = lightsUBO_->GetBindingPoints();
            auto lightArrayName = lightsUBO_->GetUBOName();
            lightParams_.resize(lights_.size());
            lightsUBO_.reset(new GLUniformBuffer(lightArrayName, sizeof(SpotLightParams) * static_cast<unsigned int>(lightParams_.size()), uniformBindingPoints));
        }

        auto nextTexture = firstTextureUnit;
        for (unsigned int i = 0; i < lights_.size(); ++i) {
            shadowMapTextureUnits[i] = nextTexture;
            nextTexture = lights_[i].UpdateLightParameters(lightParams_[i], nextTexture);
        }

        lightsUBO_->UploadData(0, sizeof(SpotLightParams) * static_cast<unsigned int>(lightParams_.size()), lightParams_.data());
        lightsUBO_->BindBuffer();
        return nextTexture;
    }
}
