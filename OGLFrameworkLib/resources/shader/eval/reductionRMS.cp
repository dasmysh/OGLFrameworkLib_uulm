// shadertype=glsl
#version 430

layout(rgba32f) uniform image2D reduceTex;

layout(local_size_x = 32, local_size_y = 16) in;
const uint localSize = gl_WorkGroupSize.x * gl_WorkGroupSize.y;

shared vec4 locRedData[localSize];

void main() {
    locRedData[gl_LocalInvocationIndex] = vec4(0.0f);

    ivec2 patchSize = ivec2(gl_NumWorkGroups.xy);
    ivec2 patchPos = ivec2(gl_LocalInvocationID.xy);
    ivec2 inPatchPos = ivec2(gl_WorkGroupID.xy);
    
    ivec2 storePos = (patchPos * patchSize) + inPatchPos;
    ivec2 origSize = imageSize(reduceTex);
    if (storePos.x >= origSize.x || storePos.y >= origSize.y) return;

    vec4 data0 = imageLoad(reduceTex, storePos);
    data0.x *= data0.x;
    data0.y *= data0.y;
    locRedData[gl_LocalInvocationIndex] = data0;
    memoryBarrierShared();
    barrier();

    uint currentSize = localSize;
    while (currentSize > 1) {
        currentSize /= 2;
        if (gl_LocalInvocationIndex >= currentSize) return;
        vec4 data1 = locRedData[gl_LocalInvocationIndex + currentSize];
        data0.x += data1.x;
        data0.y += data1.y;
        data0.z = max(data0.z, data1.z);
        data0.w += data1.w;
        locRedData[gl_LocalInvocationIndex] = data0;
        memoryBarrierShared();
        barrier();
    }

    imageStore(reduceTex, storePos, data0);
    // imageStore(reduceTex, storePos, vec4(vec2(inPatchPos.xy) / vec2(gl_NumWorkGroups.xy), vec2(patchPos.xy) / vec2(gl_WorkGroupSize.xy)));
}
