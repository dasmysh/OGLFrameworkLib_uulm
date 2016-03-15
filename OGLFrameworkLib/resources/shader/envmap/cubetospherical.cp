#version 430

#include "../coordinates.glsl"

uniform samplerCube cubeMap;
layout(rgba32f) uniform image2D sphericalTex;

const float pi = 3.14159265f;

layout(local_size_x = 32, local_size_y = 16, local_size_z = 1) in;
void main() {
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    ivec2 targetSize = imageSize(sphericalTex);
    if (pos.x >= targetSize.x || pos.y >= targetSize.y) return;
    
    vec3 normal = sphericalToCartesian(vec2((float(pos.x) / float(targetSize.x)) * 2.0f * pi, (float(pos.y) / float(targetSize.y)) * pi));
    vec4 result = vec4(texture(cubeMap, normal).rgb, 1.0f);

    imageStore(sphericalTex, pos, result);
}
