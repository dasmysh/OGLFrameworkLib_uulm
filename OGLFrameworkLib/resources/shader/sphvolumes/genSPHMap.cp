#version 430

layout(TEX) uniform image3D origTex;
layout(SPHTEX) uniform image3D sphTex0;
layout(SPHTEX) uniform image3D sphTex1;
uniform vec2 sphCoeffs;

vec4 sph(vec3 p)
{
    /*return vec4(sphCoeffs.x,
        sphCoeffs.y * p.y,
        sphCoeffs.y * p.z,
        sphCoeffs.y * p.x);*/
    return vec4(1.0f, p.y, p.z, p.x);
}

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;
void main() {
    ivec3 storePos = ivec3(gl_GlobalInvocationID.xyz);
    ivec3 sphSize = imageSize(sphTex0);
    if (storePos.x >= sphSize.x || storePos.y >= sphSize.y || storePos.z >= sphSize.z) return;
    
    ivec3 origSize = imageSize(origTex);
    ivec3 baseReadSize = ivec3(floor(vec3(origSize) / vec3(sphSize)));
    ivec3 baseReadPos = storePos * baseReadSize;
    vec3 relMidPos = 0.5f * vec3(baseReadSize);
    float halfVoxelRadius = baseReadSize.x * 0.5f;
    float halfVoxelRadSq = halfVoxelRadius * halfVoxelRadius;

    vec4 shell0 = vec4(0);
    vec4 shell1 = vec4(0);
    float numShell0 = 0.0f;
    float numShell1 = 0.0f;
    float numTot = 0.0f;
    for (int ix = 0; ix < baseReadSize.x; ++ix) {
        for (int iy = 0; iy < baseReadSize.y; ++iy) {
            for (int iz = 0; iz < baseReadSize.z; ++iz) {
                ivec3 readPos = baseReadPos + ivec3(ix, iy, iz);
                float value = imageLoad(origTex, clamp(readPos, ivec3(0), origSize - ivec3(1))).x;

                vec3 relPos = vec3(ix, iy, iz) - relMidPos;
                float distSq = dot(relPos, relPos);
                relPos = normalize(relPos);
                if (distSq > halfVoxelRadSq) {
                    numShell1 += 1.0f;
                    shell1 += value * sph(relPos);
                } else {
                    numShell0 += 1.0f;
                    shell0 += value * sph(relPos);
                }
                numTot += 1.0f;
            }
        }
    }
    shell0 /= numShell0;
    shell1 /= numShell1;

    imageStore(sphTex0, storePos, (shell0 + vec4(1.0f)) * 0.5f);
    imageStore(sphTex1, storePos, (shell1 + vec4(1.0f)) * 0.5f);
    /*imageStore(sphTex0, storePos, vec4(numShell0) / numTot);
    imageStore(sphTex1, storePos, vec4(numShell1) / numTot);*/
}
