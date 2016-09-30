// shadertype=glsl
#version 430

#include "../coordinates.glsl"
#include "../brdfs.glsl"

layout(rgba32f) uniform image2D sphericalTex;
layout(rgba32f) uniform image2D irradianceMap;

uniform ivec2 sourceRectMin;
uniform ivec2 sourceRectMax;

uniform int materialIdx;

const float pi = 3.14159265f;

layout(local_size_x = 32, local_size_y = 16, local_size_z = 1) in;
void main() {
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    ivec2 targetSize = imageSize(irradianceMap);
    if (pos.x >= targetSize.x || pos.y >= targetSize.y) return;

    ivec2 envSize = imageSize(sphericalTex);

    vec4 value = vec4(0.0f);
    if (sourceRectMin.x != 0 || sourceRectMin.y != 0) value = imageLoad(irradianceMap, pos);


    //vec4 dummy = value;

    vec3 env = sphericalToCartesian(vec2((float(pos.x) / float(targetSize.x)) * 2.0f * pi, (float(pos.y) / float(targetSize.y)) * pi));
    for (int iy = sourceRectMin.y; iy < sourceRectMax.y; ++iy) {
        for (int ix = sourceRectMin.x; ix < sourceRectMax.x; ++ix) {
            vec2 fPos = vec2(float(ix), float(iy) + 0.5f);
            vec2 sph = vec2((fPos.x / float(envSize.x)) * 2.0f * pi, (fPos.y / float(envSize.y)) * pi);
            vec3 light = sphericalToCartesian(sph);
            float cosTerm = clamp(dot(light, env), 0.0f, 1.0f);
            float weight = sin(sph.y);


            vec3 radiance = brdfI(light, env, env, materialIdx) * imageLoad(sphericalTex, ivec2(ix, iy)).rgb * cosTerm * weight;
            value += vec4(radiance, 1.0f);
        }
    }

    if (sourceRectMax.x == envSize.x && sourceRectMax.y == envSize.y) {
        value *= (4.0f * pi) / value.w;
    }

    // dummy = vec4(1.0f, 0.0f, 0.0f,1.0f);

    imageStore(irradianceMap, pos, value);
}
