#version 430

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec4 vColor;

layout(location = 0) out vec4 outputColor;

void main()
{
    outputColor = vColor;
}
