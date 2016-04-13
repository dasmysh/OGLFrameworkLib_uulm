#version 330

#include "lights.glsl"
#include "perspective.glsl"
#include "bumpMapping.glsl"

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;
uniform float bumpMultiplier;
uniform int materialIdx;

in vec3 vPos;
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBinormal;
in vec2 vTex;

out vec4 outputColor;

void main()
{
    vec3 view = normalize(camPos - vPos);
    vec3 normal = normalize(vNormal);
    vec3 tangent = normalize(vTangent);
    vec3 binormal = normalize(vBinormal);
    vec3 texColor = texture(diffuseTex, vTex).rgb;

    normal = bumpMapNormal(bumpTex, vTex, normal, tangent, binormal, bumpMultiplier);


    // normal = bump.xyz;

    vec3 intensity = lightIntensityI(vPos, normal, view, materialIdx);
    // outputColor = vec4(vec3(float(materialIdx + 1) / 3.0f), 1.0f);
    outputColor = vec4(texColor * intensity, 1.0f);
    // outputColor = vec4(vec3(shadow(vPos, 0)), 1.0f);
}
