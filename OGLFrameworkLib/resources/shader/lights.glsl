
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

uniform sampler2D shadowTextures[NUM_LIGHTS];

layout(std140) uniform lightsBuffer
{
    Light lights[NUM_LIGHTS];
};

vec2 shadow(vec3 worldPos, float NdotL, int lightIdx) {
    vec4 shadowPos = lights[lightIdx].viewProjection * vec4(worldPos, 1.0f);
    vec2 shadowTex = texture(shadowTextures[lightIdx], shadowPos.xy / shadowPos.w).xy;

    float bias = 0.002*tan(acos(NdotL));
    bias = clamp(bias, 0.0f, 0.02f);

    float t = distance(lights[lightIdx].position.xyz, worldPos) - bias;

    float mean = shadowTex.x;
    if (t <= mean) return vec2(1.0f, mean);

    float variance = shadowTex.y - (mean * mean);
    variance = max(variance, 0.0001f);

    float mmd = t - mean;
    float pmax = variance / (variance + mmd * mmd);
    float p = float(t <= mean);
    return vec2(clamp(max(p, pmax), 0.0f, 1.0f), mean);
}

/*vec4 pointLightIntensitySingle(vec3 position, vec3 normal, vec3 view, int matIdx, int lightIdx, out float smDepth) {
    vec3 light = vec3(lights[lightIdx].position) - position;
    float lDist = length(light);
    light /= lDist;

    float curve = min(pow(lDist / lights[lightIdx].farZ, 6.0), 1.0);
    float distAttenuation = mix(1.0 / (1.0 + lights[lightIdx].distAttenuation * lDist * lDist), 0.0, curve);
    float NdotL = clamp(dot(light, normal), 0.0f, 1.0f);
    float attenuation = NdotL * distAttenuation;
    vec3 color = shadow(position, NdotL, lightIdx, smDepth) * vec3(lights[lightIdx].intensity) * brdf(light, normal, view, matIdx) * attenuation;

    return vec4(color, distAttenuation);
}

vec4 spotLightIntensitySingle(vec3 position, vec3 normal, vec3 view, int matIdx, int lightIdx, out float smDepth) {
    vec3 light = vec3(lights[lightIdx].position) - position;
    float lDist = length(light);
    light /= lDist;

    vec4 result = vec4(0.0f);
    float spot = dot(vec3(lights[lightIdx].direction), -light);
    float NdotL = clamp(dot(light, normal), 0.0f, 1.0f);
    float shadow = shadow(position, NdotL, lightIdx, smDepth);
    if (spot > lights[lightIdx].angFalloffStart) {
        float curve = min(pow(lDist / lights[lightIdx].farZ, 6.0), 1.0);
        float distAttenuation = mix(1.0 / (1.0 + lights[lightIdx].distAttenuation * lDist * lDist), 0.0, curve);
        spot = clamp((spot - lights[lightIdx].angFalloffStart) / lights[lightIdx].angFalloffWidth, 0.0f, 1.0f);
        result.w = distAttenuation * spot;
        float attenuation = NdotL * result.w;
        result.xyz = shadow * vec3(lights[lightIdx].intensity) * brdf(light, normal, view, matIdx) * attenuation;
    }

    return result;
}*/

float pointLightAttenuation(vec3 light, float lDist, int lightIdx) {
    float curve = min(pow(lDist / lights[lightIdx].farZ, 6.0), 1.0);
    return mix(1.0 / (1.0 + lights[lightIdx].distAttenuation * lDist * lDist), 0.0, curve);
}

float spotLightAttenuation(vec3 light, float lDist, int lightIdx) {
    float spot = dot(vec3(lights[lightIdx].direction), -light);
    float ltAttenuation = 0.0f;
    if (spot > lights[lightIdx].angFalloffStart) {
        float distAttenuation = pointLightAttenuation(light, lDist, lightIdx);
        spot = clamp((spot - lights[lightIdx].angFalloffStart) / lights[lightIdx].angFalloffWidth, 0.0f, 1.0f);
        ltAttenuation = spot * distAttenuation;
    }
    return ltAttenuation;
}

#define LIGHT_ATT_PASTE(type, fname, light, lDist, lightIdx) type ## fname ## (light, lDist, lightIdx)
#define LIGHT_ATT_PASTE2(type, fname, light, lDist, lightIdx) LIGHT_ATT_PASTE(type, fname, light, lDist, lightIdx)
#define lightAttenuation(light, lDist, lightIdx)  LIGHT_ATT_PASTE2(LIGHT_TYPE, LightAttenuation, light, lDist, lightIdx)

vec4 lightSingleBRDF(vec3 position, vec3 normal, vec3 view, Material mat, int lightIdx) {
    vec3 light = vec3(lights[lightIdx].position) - position;
    float lDist = length(light);
    light /= lDist;

    float NdotL = clamp(dot(light, normal), 0.0f, 1.0f);
    float shadow = shadow(position, NdotL, lightIdx).r;
    float ltAttenuation = lightAttenuation(light, lDist, lightIdx);
    float attenuation = NdotL * ltAttenuation * shadow;

    vec3 color = vec3(lights[lightIdx].intensity) * brdf(light, normal, view, mat) * attenuation;

    return vec4(color, ltAttenuation);
}

vec4 lightSingleBRDFI(vec3 position, vec3 normal, vec3 view, int matIdx, int lightIdx) {
    return lightSingleBRDF(position, normal, view, materials[matIdx], lightIdx);
}

/*#define LIGHT_PASTE(type, fname, position, normal, view, matIdx, lightIdx, smDepth) type ## fname ## (position, normal, view, matIdx, lightIdx, smDepth)
#define LIGHT_PASTE2(type, fname, position, normal, view, matIdx, lightIdx, smDepth) LIGHT_PASTE(type, fname, position, normal, view, matIdx, lightIdx, smDepth)
#define lightIntensitySingle(position, normal, view, matIdx, lightIdx, smDepth)  LIGHT_PASTE2(LIGHT_TYPE, LightIntensitySingle, position, normal, view, matIdx, lightIdx, smDepth)*/


vec3 lightIntensity(vec3 position, vec3 normal, vec3 view, int matIdx) {
    vec3 intensity = vec3(0.0f);
    for (int i = 0; i < NUM_LIGHTS; ++i) {
        intensity += vec3(lightSingleBRDFI(position, normal, view, matIdx, i));
    }
    return intensity;
}
