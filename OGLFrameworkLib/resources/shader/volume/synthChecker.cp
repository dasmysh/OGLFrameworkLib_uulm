#version 430
layout(r32f) uniform image3D resultImg;
uniform uvec3 checkerSize;
uniform uvec3 offset;

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;
void main() {
    ivec3 storePos = ivec3(gl_GlobalInvocationID.xyz);
    ivec3 resultSize = imageSize(resultImg);
    if (storePos.x >= resultSize.x || storePos.y >= resultSize.y || storePos.z >= resultSize.z) return;

    float val = 0.0f;
    uvec3 storePosU = uvec3(storePos) + offset;
    uvec3 checker = storePosU / checkerSize;
    if (checker.z % 2 == 1) checker.y += 1;
    if (checker.y % 2 == 1) checker.x += 1;
    if (checker.x % 2 == 1) val = 1.0f;

    imageStore(resultImg, storePos, vec4(val));
}
