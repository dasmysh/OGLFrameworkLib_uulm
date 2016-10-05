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
#include <core/serializationHelper.h>

namespace cgu {


    SpotLight::SpotLight(const glm::vec3& intensity, float fov, const glm::vec3& pos, std::unique_ptr<ShadowMap> shadowMap, ApplicationBase* app) :
        camera_(GLFW_MOUSE_BUTTON_RIGHT, fov, shadowMap->GetSize(), 5.0f, 15.0f, pos, app->GetUBOBindingPoints()),
        falloffWidth_(0.05f),
        intensity_(intensity),
        attenuation_(1.0f / 128.0f),
        bias_(-0.01f),
        shadowMap_(std::move(shadowMap)),
        application_(app)
    {
    }

    SpotLight::SpotLight(const glm::vec3& intensity, float fov, const glm::vec3& pos, std::unique_ptr<GLRenderTarget>&& shadowMapRT,
        const std::shared_ptr<GPUProgram>& smProgram, const std::shared_ptr<GPUProgram>& filterProgram, ApplicationBase* app) :
        SpotLight(intensity, fov, pos, std::make_unique<ShadowMap>(std::move(shadowMapRT), *this, smProgram, filterProgram, app), app)
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
        SpotLight(rhs.intensity_, rhs.GetCamera().GetFOV(), rhs.GetCamera().GetPosition(),
        std::make_unique<GLRenderTarget>(*rhs.GetShadowMap()->GetShadowTarget()), rhs.GetShadowMap()->GetShadowMappingProgram(),
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
    int SpotLight::UpdateLightParameters(SpotLightParams& params, int nextTextureUnit, std::vector<int>& shadowMapTextureUnits, int firstUnitsEntry) const
    {
        params.position = glm::vec4(camera_.GetPosition(), 1.0f);
        params.direction = glm::vec4(glm::normalize(glm::vec3(0.0f) - camera_.GetPosition()), 1.0f);
        params.intensity = glm::vec4(intensity_, 1.0f);
        params.angFalloffStart = glm::cos(0.5f * camera_.GetFOV());
        params.angFalloffWidth = falloffWidth_;
        params.distAttenuation = attenuation_;
        params.farZ = 100.0f; // camera_.GetFarZ();
        params.viewProjection = shadowMap_->GetViewProjectionTextureMatrix(camera_.GetViewMatrix(), camera_.GetProjMatrix());

        for (const auto& tex : shadowMap_->GetShadowTarget()->GetTextures()) {
            shadowMapTextureUnits[firstUnitsEntry++] = nextTextureUnit;
            tex->ActivateTexture(GL_TEXTURE0 + nextTextureUnit++);
        }

        return nextTextureUnit;
    }

    void SpotLight::SaveParameters(std::ostream& ostr) const
    {
        auto view = camera_.GetViewMatrix();
        auto proj = camera_.GetProjMatrix();

        serializeHelper::write(ostr, std::string("SpotLight"));
        serializeHelper::write(ostr, VERSION);
        serializeHelper::write(ostr, view);
        serializeHelper::write(ostr, proj);
        serializeHelper::write(ostr, intensity_);
        serializeHelper::write(ostr, falloffWidth_);
        serializeHelper::write(ostr, attenuation_);
    }

    void SpotLight::LoadParameters(std::istream& istr)
    {
        std::string clazzName;
        unsigned int version;
        serializeHelper::read(istr, clazzName);
        if (clazzName != "SpotLight") throw std::runtime_error("Serialization Error: wrong class.");
        serializeHelper::read(istr, version);
        if (version > VERSION) throw std::runtime_error("Serialization Error: wrong version.");

        glm::mat4 view, proj;
        serializeHelper::read(istr, view);
        serializeHelper::read(istr, proj);
        serializeHelper::read(istr, intensity_);
        serializeHelper::read(istr, falloffWidth_);
        serializeHelper::read(istr, attenuation_);
        camera_.ResetCamera(proj, view);
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
        assert(shadowMapTextureUnits.size() == lights_.size() * lights_[0].GetShadowMap()->GetShadowTarget()->GetTextures().size());

        if (lights_.size() != lightParams_.size()) {
            auto uniformBindingPoints = lightsUBO_->GetBindingPoints();
            auto lightArrayName = lightsUBO_->GetUBOName();
            lightParams_.resize(lights_.size());
            lightsUBO_.reset(new GLUniformBuffer(lightArrayName, sizeof(SpotLightParams) * static_cast<unsigned int>(lightParams_.size()), uniformBindingPoints));
        }

        auto nextTexture = firstTextureUnit;
        for (unsigned int i = 0; i < lights_.size(); ++i) {
            // shadowMapTextureUnits[i] = nextTexture;
            auto nextUnitEntry = i * static_cast<int>(lights_[0].GetShadowMap()->GetShadowTarget()->GetTextures().size());
            nextTexture = lights_[i].UpdateLightParameters(lightParams_[i], nextTexture, shadowMapTextureUnits, nextUnitEntry);
        }

        lightsUBO_->UploadData(0, sizeof(SpotLightParams) * static_cast<unsigned int>(lightParams_.size()), lightParams_.data());
        lightsUBO_->BindBuffer();
        return nextTexture;
    }

    void SpotLightArray::SaveParameters(std::ostream& ostr) const
    {
        serializeHelper::write(ostr, std::string("SpotLightArray"));
        serializeHelper::write(ostr, VERSION);

        for (const auto& lgt : lights_) lgt.SaveParameters(ostr);
    }

    void SpotLightArray::LoadParameters(std::istream& istr)
    {
        std::string clazzName;
        unsigned int version;
        serializeHelper::read(istr, clazzName);
        if (clazzName != "SpotLightArray") throw std::runtime_error("Serialization Error: wrong class.");
        serializeHelper::read(istr, version);
        if (version > VERSION) throw std::runtime_error("Serialization Error: wrong version.");

        for (auto& lgt : lights_) lgt.LoadParameters(istr);
    }
}
