#version 430

layout(rgba32f) uniform image3D origTex;
layout(rgba32f) uniform image3D downsizedTex;

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;
void main() {
    ivec3 storePos = ivec3(gl_GlobalInvocationID.xyz);
    ivec3 origSize = ivec3(imageSize(origTex));
    ivec3 downsizedSize = ivec3(imageSize(downsizedTex));
    if (storePos.x >= downsizedSize.x || storePos.y >= downsizedSize.y || storePos.z >= downsizedSize.z) return;

    ivec3 readBasePos = storePos * 2;
    ivec3 readPos[8];
    readPos[0] = readBasePos + ivec3(0, 0, 0);
    readPos[1] = readBasePos + ivec3(0, 0, 1);
    readPos[2] = readBasePos + ivec3(0, 1, 0);
    readPos[3] = readBasePos + ivec3(0, 1, 1);
    readPos[4] = readBasePos + ivec3(1, 0, 0);
    readPos[5] = readBasePos + ivec3(1, 0, 1);
    readPos[6] = readBasePos + ivec3(1, 1, 0);
    readPos[7] = readBasePos + ivec3(1, 1, 1);

    float avgVals[8];

    for (int i = 0; i < 8; ++i) {
        avgVals[i] = imageLoad(origTex, clamp(readPos[i], ivec3(0), origSize)).x;
    }

    int n = 8;
    do {
        int newn = 1;
        for (int i = 0; i < n - 1; ++i) {
            if (avgVals[i] > avgVals[i + 1]) {
                float tmpVal = avgVals[i];
                avgVals[i] = avgVals[i + 1];
                avgVals[i + 1] = tmpVal;
                newn = i + 1;
            } // ende if
        } // ende for
        n = newn;
    } while (n > 1);

    float avg = (avgVals[3] + avgVals[4]) * 0.5f;
    imageStore(downsizedTex, storePos, vec4(avg, avg, avg, 0.0f));
}
