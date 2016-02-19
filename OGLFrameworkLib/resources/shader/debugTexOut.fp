#version 330

uniform sampler2D sourceTex;

in vec2 vTex;
out vec4 result;

void main()
{
    result = texture(sourceTex, vTex);
}
