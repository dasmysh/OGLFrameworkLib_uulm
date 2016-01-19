#version 450

uniform sampler3D volume;
uniform sampler1D transferFunc;
layout(rgba32f) uniform image2D back;
uniform float stepSize = 1.0f / 128.0f;
uniform float lodLevel = 0.0f;

in vec4 gl_FragCoord;

in vec2 vTexPosition;
in vec3 vVolPosition;

layout(location = 0) out vec4 outputColor;

#define REF_SAMPLING_INTERVAL 150.0

void main()
{
    ivec2 imgCoords = ivec2(gl_FragCoord.xy);
    vec4 backTex = imageLoad(back, imgCoords);
    float overShoot = 0.0f;

    // Setup ray param
    vec3 rayStart = vVolPosition;
    vec3 rayDir = backTex.xyz - rayStart;

    float t1 = min(length(rayDir), length(vec3(1)));
    rayDir /= t1;

    // Reset Color and Alpha
    vec4 colorAccValue = vec4(0.0f);
    vec3 C = colorAccValue.rgb;
    float A = colorAccValue.a;

    float t = overShoot;
    while (t < t1 && A < 1.0) {
        // World Space Position
        vec3 p = rayStart + rayDir * t;

        // Intensity value of volume data
        float s = textureLod(volume, p, lodLevel).r;

        // Transfer function lookup
        vec4 color = texture(transferFunc, s);
        // color.a *= stepSize * 100;
        color.a = 1.0 - pow(1.0 - color.a, stepSize * REF_SAMPLING_INTERVAL);

        C += (1 - A) * color.a * color.rgb;
        A += (1 - A) * color.a;

        t += stepSize;
    }

    outputColor = vec4(C, 1);
}
