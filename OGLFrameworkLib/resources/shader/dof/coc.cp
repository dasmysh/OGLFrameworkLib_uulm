#version 430

#include "../coordinates.glsl"

uniform sampler2D colorTex;
uniform sampler2D depthTex;
layout(rgba32f) uniform image2D targetTex;

uniform float focusZ;
uniform float scale;
uniform vec3 clipInfo;

layout(local_size_x = 32, local_size_y = 16, local_size_z = 1) in;
void main() {
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    ivec2 targetSize = imageSize(targetTex);
    if (pos.x >= targetSize.x || pos.y >= targetSize.y) return;

    vec4 result;
    result.rgb = texelFetch(colorTex, pos, 0).rgb;
    float z = reconstructLinearZ(texelFetch(depthTex, pos, 0).r, clipInfo);
    result.a = (z - focusZ) * scale;
    result.a = (result.a * 0.5f) + 0.5f;
    // result.a = z;

    imageStore(targetTex, pos, result);
    // imageStore(targetTex, pos, vec4(result.rgb, 1));
}
