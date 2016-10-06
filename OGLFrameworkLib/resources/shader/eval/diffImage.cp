// shadertype=glsl
#version 430

layout(rgba8) uniform image2D origTex;
layout(rgba8) uniform image2D cmpTex;
layout(rgba8) uniform image2D resultTex;
layout(rgba32f) uniform image2D statsTex;

layout(local_size_x = 32, local_size_y = 16) in;
void main() {
    ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
    ivec2 origSize = imageSize(origTex);
    if (storePos.x >= origSize.x || storePos.y >= origSize.y) return;

    vec4 orig = imageLoad(origTex, storePos);
    vec4 cmp = imageLoad(cmpTex, storePos);
    vec3 diff = abs(vec3(cmp - orig));
    imageStore(resultTex, storePos, vec4(diff, 1.0f));

    float rmsAvg = dot(diff, vec3(1.0f)) / 3.0f;
    float rmsMax = max(max(diff.r, diff.g), diff.b);
    float maxError = rmsMax;
    float errorPixel = 0.0f;
    if (diff.r > 0.0001f || diff.g > 0.0001f || diff.b > 0.0001f) errorPixel = 1.0f;
    imageStore(statsTex, storePos, vec4(rmsAvg, rmsMax, maxError, errorPixel));
}
