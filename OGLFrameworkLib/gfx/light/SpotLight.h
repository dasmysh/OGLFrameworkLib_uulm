/**
 * @file   SpotLight.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.12.17
 *
 * @brief  Declaration of a spot light.
 */

#ifndef SPOTLIGHT_H
#define SPOTLIGHT_H

#include "main.h"
#include "gfx/ArcballCamera.h"

namespace cgu {

    class GLRenderTarget;
    class GPUProgram;
    class ShadowMap;

    struct SpotLightParams
    {
        /** Holds the lights position. */
        glm::vec4 position;
        /** Holds the lights direction. */
        glm::vec4 direction;
        /** Holds the lights intensity (power per solid angle). */
        glm::vec4 intensity;
        /** Holds the start of the angular falloff. */
        float angFalloffStart;
        /** Holds the width of the angular falloff. */
        float angFalloffWidth;
        /** Holds the distance attenuation. */
        float distAttenuation;
        /** Holds the z value of the far plane. */
        float farZ;
        /** Holds the transformation matrix for the shadow map. */
        glm::mat4 viewProjection;
    };

    class SpotLight
    {
    public:
        SpotLight(const glm::vec3&  intensity, float fov, const glm::vec3& pos, std::unique_ptr<GLRenderTarget>&& shadowMapRT,
            const std::shared_ptr<GPUProgram>& smProgram, const std::shared_ptr<GPUProgram>& filterProgram, ApplicationBase* app);
        SpotLight(const glm::vec3&  intensity, float fov, const glm::vec3& pos, const glm::uvec2& smSize, ApplicationBase* app);
        SpotLight(const SpotLight&);
        SpotLight& operator=(const SpotLight&);
        SpotLight(SpotLight&&);
        SpotLight& operator=(SpotLight&&);
        ~SpotLight();

        void Resize(const glm::uvec2& shadowMapSize);
        bool HandleKeyboard(int key, int scancode, int action, int mods, GLWindow* sender);
        bool HandleMouse(int button, int action, int mods, float mouseWheelDelta, GLWindow* sender);
        void UpdateLight();
        int UpdateLightParameters(SpotLightParams& params, int nextTextureUnit, std::vector<int>& shadowMapTextureUnits, int firstUnitsEntry) const;
        void SetFOV(float fov) { camera_.SetFOV(fov); }
        /** Returns the view matrix of the light. */
        const glm::mat4& GetViewMatrix() const { return camera_.GetViewMatrix(); }
        /** Returns the lights position. */
        const glm::vec3& GetPosition() const { return camera_.GetPosition(); }
        /** Returns the lights intensity (const). */
        const glm::vec3& GetIntensity() const { return intensity_; }
        /** Returns the lights intensity. */
        glm::vec3& GetIntensity() { return intensity_; }
        /** Returns the camera view. */
        const ArcballCamera& GetCamera() const { return camera_; }
        /** Returns the shadow map. */
        ShadowMap* GetShadowMap() const { return shadowMap_.get(); }

        void SetPosition(const glm::vec3& position) { camera_.SetPosition(position); }

        void SaveParameters(std::ostream& ostr) const;
        void LoadParameters(std::istream& istr);

    private:
        static const unsigned int VERSION = 1;

        SpotLight(const glm::vec3&  intensity, float fov, const glm::vec3& pos, std::unique_ptr<ShadowMap> shadowMap, ApplicationBase* app);

        /** Holds the lights camera object. */
        ArcballCamera camera_;
        /** Holds the falloff. */
        float falloffWidth_;
        /** Holds the lights intensity (power per solid angle). */
        glm::vec3 intensity_;
        /** Holds the lights attenuation. */
        float attenuation_;
        /** Holds the shadow map bias. */
        float bias_;
        /** Holds the shadow map. */
        std::unique_ptr<ShadowMap> shadowMap_;
        /** Holds the application base object. */
        ApplicationBase* application_;
    };

    class SpotLightArray
    {
    public:
        SpotLightArray(const std::string& lightArrayName, ShaderBufferBindingPoints* uniformBindingPoints);
        ~SpotLightArray();

        int SetLightParameters(int firstTextureUnit, std::vector<int>& shadowMapTextureUnits);

        /** Returns the managed lights array (const). */
        const std::vector<SpotLight>& GetLights() const { return lights_; }
        /** Returns the managed lights array. */
        std::vector<SpotLight>& GetLights() { return lights_; }

        void SaveParameters(std::ostream& ostr) const;
        void LoadParameters(std::istream& istr);

    private:
        static const unsigned int VERSION = 1;
        /** Holds the lights. */
        std::vector<SpotLight> lights_;
        /** Holds the lights parameters. */
        std::vector<SpotLightParams> lightParams_;
        /** Holds the lights uniform buffer. */
        std::unique_ptr<GLUniformBuffer> lightsUBO_;
    };
}

#endif // SPOTLIGHT_H
