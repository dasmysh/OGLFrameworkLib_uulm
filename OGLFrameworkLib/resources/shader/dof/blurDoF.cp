#version 430

uniform sampler2D sourceTex;
layout(rgba32f) uniform image2D targetFrontTex;
layout(rgba32f) uniform image2D targetBackTex;

uniform int maxCoCRadius;
uniform int frontBlurRadius;
uniform float invFrontBlurRadius;

#ifdef HORIZONTAL
    const ivec2 direction = ivec2(1, 0);
#else
    const ivec2 direction = ivec2(0, 1);
    uniform sampler2D sourceFrontTex;
#endif

const int KERNEL_SIZE = 6;
const float kernel[KERNEL_SIZE + 1] = float[KERNEL_SIZE + 1] (1.0f, 1.0f, 0.9f, 0.75f, 0.6f, 0.5f, 0.0f);

layout(local_size_x = 32, local_size_y = 16, local_size_z = 1) in;
void main() {
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    ivec2 targetSize = imageSize(targetFrontTex);

    if (pos.x >= targetSize.x || pos.y >= targetSize.y) return;

    vec4 backResult = vec4(0.0f);
    float backWeightSum = 0.0f;
    vec4 frontResult = vec4(0.0f);
    float frontWeightSum = 0.0f;

    ivec2 sourcePos = ivec2(pos * (direction * (SIZE_FACTOR - 1) + ivec2(1)));
    float packedCoC = texelFetch(sourceTex, sourcePos, 0).a;
    float coc = ((packedCoC * 2.0f) - 1.0f) * float(maxCoCRadius);
    float frontCoC = clamp(coc * float(SIZE_FACTOR), 0.0f, 1.0f);

    for (int delta = -maxCoCRadius; delta <= maxCoCRadius; ++delta) {
        ivec2 samplePos = sourcePos + (direction * delta);
        vec4 backInput = texelFetch(sourceTex, clamp(samplePos, ivec2(0), textureSize(sourceTex, 0) - ivec2(1)), 0);
        float sampleCoC = ((backInput.a * 2.0f) - 1.0f) * float(maxCoCRadius);

        int filterPos = int(float(abs(delta) * (KERNEL_SIZE - 1)) / (0.001f + abs(sampleCoC * 0.8f)));
        float wNorm = (sampleCoC < 0.25) ? kernel[clamp(filterPos, 0, KERNEL_SIZE)] : 0.0f;
        float weight = mix(wNorm, 1.0, frontCoC);
        backWeightSum  += weight;
        backResult.rgb += backInput.rgb * weight;

        vec4 frontInput;
#ifdef HORIZONTAL
        float fIFact = ((abs(delta) <= sampleCoC) ? 1.0f : 0.0f);
        frontInput.a = fIFact * clamp(sampleCoC * invFrontBlurRadius * float(SIZE_FACTOR), 0.0f, 1.0f);
        frontInput.a *= frontInput.a; frontInput.a *= frontInput.a;
        frontInput.rgb = backInput.rgb * frontInput.a;
#else
        frontInput = texelFetch(sourceFrontTex, clamp(samplePos, ivec2(0), textureSize(sourceFrontTex, 0) - ivec2(1)), 0);
#endif
        weight = float(abs(delta) < frontBlurRadius);
        frontResult += frontInput * weight;
        frontWeightSum += weight;
    }

#ifdef HORIZONTAL
    backResult.a = packedCoC;
#else
    backResult.a = 1.0f;
#endif

    backResult.rgb /= backWeightSum;
    frontResult /= max(frontWeightSum, 0.00001f);

    imageStore(targetFrontTex, pos, frontResult);
    imageStore(targetBackTex, pos, backResult);
}
