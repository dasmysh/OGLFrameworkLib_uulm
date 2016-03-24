#version 330

#include "perspective.glsl"

in vec4 worldPos;
out vec4 moments;

void main()
{
    float depth = distance(worldPos.xyz, camPos);
    float m1 = depth;
    float m2 = depth * depth;
    float dx = dFdx(m1);
    float dy = dFdy(m1);
    m2 += 0.25f * (dx*dx + dy*dy);

    moments  = vec4(m1, m2, 0.0f, 1.0f);
}
