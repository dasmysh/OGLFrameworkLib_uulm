#version 330

smooth in vec2 vTex;

out vec4 diffuseColor;

uniform sampler2D guiTex;

void main() {
  vec4 texVal = vec4(texture(guiTex, vTex).xyz, 1.0f);

  diffuseColor = texVal;
}
