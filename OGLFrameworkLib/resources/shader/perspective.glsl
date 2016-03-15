
layout(std140) uniform perspectiveTransform
{
    mat4 mat_vp;
    vec3 camPos;
};

uniform mat3 normalMatrix;
uniform mat4 modelMatrix;
