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
        SpotLight(const glm::vec3&  intensity, float fov, const glm::vec3& pos, const glm::uvec2& smSize, ApplicationBase* app);
        SpotLight(const SpotLight&);
        SpotLight& operator=(const SpotLight&);
        SpotLight(SpotLight&&);
        SpotLight& operator=(SpotLight&&);
        ~SpotLight();

        void Resize(const glm::uvec2& shadowMapSize);
        bool HandleKeyboard(unsigned int vkCode, bool bKeyDown, BaseGLWindow* sender);
        bool HandleMouse(unsigned int buttonAction, float mouseWheelDelta, BaseGLWindow* sender);
        void UpdateLight();
        int UpdateLightParameters(SpotLightParams& params, int nextTextureUnit) const;
        /** Returns the view matrix of the light. */
        const glm::mat4& GetViewMatrix() const { return camera.GetViewMatrix(); }
        /** Returns the lights position. */
        const glm::vec3& GetPosition() const { return camera.GetPosition(); }
        /** Returns the lights intensity (const). */
        const glm::vec3& GetIntensity() const { return intensity; }
        /** Returns the lights intensity. */
        glm::vec3& GetIntensity() { return intensity; }
        /** Returns the camera view. */
        const ArcballCamera& GetCamera() const { return camera; }
        /** Returns the shadow map. */
        ShadowMap* GetShadowMap() const { return shadowMap.get(); }

    private:
        /** Holds the lights camera object. */
        ArcballCamera camera;
        /** Holds the falloff. */
        float falloffWidth;
        /** Holds the lights intensity (power per solid angle). */
        glm::vec3 intensity;
        /** Holds the lights attenuation. */
        float attenuation;
        /** Holds the lights maximum range. */
        // float farPlane;
        /** Holds the shadow map bias. */
        float bias;
        /** Holds the shadow map. */
        std::unique_ptr<ShadowMap> shadowMap;
        /** Holds the application base object. */
        ApplicationBase* application;
    };

    class SpotLightArray
    {
    public:
        SpotLightArray(const std::string& lightArrayName, ShaderBufferBindingPoints* uniformBindingPoints);
        ~SpotLightArray();

        int SetLightParameters(int firstTextureUnit, std::vector<int>& shadowMapTextureUnits);

        /** Returns the managed lights array (const). */
        const std::vector<SpotLight>& GetLights() const { return lights; }
        /** Returns the managed lights array. */
        std::vector<SpotLight>& GetLights() { return lights; }

    private:
        /** Holds the lights. */
        std::vector<SpotLight> lights;
        /** Holds the lights parameters. */
        std::vector<SpotLightParams> lightParams;
        /** Holds the lights uniform buffer. */
        std::unique_ptr<GLUniformBuffer> lightsUBO;
    };
}

#endif // SPOTLIGHT_H
