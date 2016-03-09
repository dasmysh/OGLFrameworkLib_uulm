
#include "brdfs.glsl"

#ifndef LIGHT_TYPE
    #define LIGHT_TYPE point
#endif

struct Light
{
    vec4 position;
    vec4 direction;
    vec4 intensity;
    float angFalloffStart;
    float angFalloffWidth;
    float distAttenuation;
    float farZ;
    mat4 viewProjection;
};

uniform sampler2DShadow shadowTextures[NUM_LIGHTS];

layout(std140) uniform lightsBuffer
{
    Light lights[NUM_LIGHTS];
};

float shadow(vec3 worldPos, int i) {
    vec4 shadowPos = lights[i].viewProjection * vec4(worldPos, 1.0f);
    shadowPos.z = shadowPos.z - 0.0001f;
    shadowPos.xyz /= shadowPos.w;
    return texture(shadowTextures[i], shadowPos.xyz);
}

vec3 pointLightIntensity(vec3 position, vec3 normal, vec3 view, int matIdx) {
    vec3 intensity = vec3(0.0f);
    for (int i = 0; i < NUM_LIGHTS; ++i) {
        vec3 light = vec3(lights[i].position) - position;
        float lDist = length(light);
        light /= lDist;

        float cosTerm = clamp(dot(light, normal), 0.0f, 1.0f);
        intensity += shadow(position, i) * vec3(lights[i].intensity) * brdf(light, normal, view, matIdx) * cosTerm;
    }
    return intensity;
}

vec3 spotLightIntensity(vec3 position, vec3 normal, vec3 view, int matIdx) {
    vec3 intensity = vec3(0.0f);
    for (int i = 0; i < NUM_LIGHTS; ++i) {
        vec3 light = vec3(lights[i].position) - position;
        float lDist = length(light);
        light /= lDist;

        float cosTerm = clamp(dot(light, normal), 0.0f, 1.0f);
        intensity += shadow(position, i) * vec3(lights[i].intensity) * brdf(light, normal, view, matIdx) * cosTerm;
    }
    return intensity;
}

#define LIGHT_PASTE(type, fname, position, normal, view, matIdx) type ## fname ## (position, normal, view, matIdx)
#define LIGHT_PASTE2(type, fname, position, normal, view, matIdx) LIGHT_PASTE(type, fname, position, normal, view, matIdx)
#define lightIntensity(position, normal, view, matIdx)  LIGHT_PASTE2(LIGHT_TYPE, LightIntensity, position, normal, view, matIdx)
