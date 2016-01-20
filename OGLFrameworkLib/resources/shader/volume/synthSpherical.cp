#version 430
layout(r32f) uniform image3D resultImg;
uniform vec3 sphereCenter;
uniform vec3 sphereScale;
uniform uvec3 offset;

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;
void main() {
    ivec3 storePos = ivec3(gl_GlobalInvocationID.xyz);
    ivec3 resultSize = imageSize(resultImg);
    if (storePos.x >= resultSize.x || storePos.y >= resultSize.y || storePos.z >= resultSize.z) return;

    uvec3 storePosU = uvec3(storePos) + offset;
    vec3 volPos = vec3(storePosU);
    float val = length((volPos - sphereCenter) * sphereScale);

    imageStore(resultImg, storePos, vec4(val));
}
