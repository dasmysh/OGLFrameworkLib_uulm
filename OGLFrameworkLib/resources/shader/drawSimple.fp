#version 430

uniform vec4 color;

layout(location = 0) in vec3 vPos;

layout(location = 0) out vec4 outputColor;

void main()
{
    outputColor = color;
}
