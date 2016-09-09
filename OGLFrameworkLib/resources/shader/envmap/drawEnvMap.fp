// shadertype=glsl
#version 430

#include "../coordinates.glsl"

#define M_PI 3.1415926535897932384626433832795f

uniform sampler2D envMapTex;
uniform mat4 vpInv;
uniform vec3 camPos;

in vec2 vTex;

out vec4 outputColor;

void main() {
    vec3 coordsNDC = vec3((2.0f * vTex) - vec2(1.0f), -1.0f);
    vec4 coordsWorld = vpInv * vec4(coordsNDC, 1.0f);

    vec3 viewDir = normalize((coordsWorld.xyz / coordsWorld.w) - camPos);

    vec2 coordsEnv = cartesianToSpherical(viewDir);
    coordsEnv.x = (M_PI + coordsEnv.x) / (2.0f * M_PI);
    coordsEnv.y = 1.0f - (coordsEnv.y / M_PI);

    outputColor = vec4(texture(envMapTex, coordsEnv).rgb, 1.0f);
}
