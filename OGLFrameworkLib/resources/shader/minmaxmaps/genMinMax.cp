#version 430

layout(AVGTEX) uniform image3D origTex;
layout(MMTEX) uniform image3D minMaxTex;

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;
void main() {
    ivec3 storePos = ivec3(gl_GlobalInvocationID.xyz);
    ivec3 minMaxSize = imageSize(minMaxTex);
    if (storePos.x >= minMaxSize.x || storePos.y >= minMaxSize.y || storePos.z >= minMaxSize.z) return;
    
    ivec3 origSize = imageSize(origTex);
    ivec3 baseReadSize = ivec3(floor(vec3(origSize) / vec3(minMaxSize)));
    ivec3 baseReadPos = storePos * baseReadSize;

    float minValue = 1.0f;
    float maxValue = 0.0f;
    for (int ix = 0; ix < baseReadSize.x; ++ix) {
        for (int iy = 0; iy < baseReadSize.y; ++iy) {
            for (int iz = 0; iz < baseReadSize.z; ++iz) {
                ivec3 readPos = baseReadPos + ivec3(ix, iy, iz);
                float value = imageLoad(origTex, clamp(readPos, ivec3(0), origSize - ivec3(1))).x;
                minValue = min(minValue, value);
                maxValue = max(maxValue, value);
            }
        }
    }

    imageStore(minMaxTex, storePos, vec4(minValue, maxValue, 0, 0));
}
