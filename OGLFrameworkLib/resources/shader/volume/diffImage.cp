#version 430

layout(rgba8) uniform image2D origTex;
layout(rgba8) uniform image2D cmpTex;
layout(rgba8) uniform image2D resultTex;
layout(r32f) uniform image2D resultTexDeltaE;

vec3 rgbToCIE(vec3 c)
{
    vec3 result;
    result.x = (0.4124564 * c.r) + (0.3575761 * c.g) + (0.1804375 * c.b);
    result.y = (0.2126729 * c.r) + (0.7151522 * c.g) + (0.0721750 * c.b);
    result.z = (0.0193339 * c.r) + (0.1191920 * c.g) + (0.9503041 * c.b);
    return result;
}

vec3 cieToLAB(vec3 c)
{
    vec3 result;
    float yFact = pow(c.y / 100.0f, 1.0f / 3.0f);
    result.x = (116.0f * yFact) - 16.0f;
    result.y = 500.0f * (pow(c.x / 95.047f, 1.0f / 3.0f) - yFact);
    result.z = 200.0f * (yFact - pow(c.z / 108.883f, 1.0f / 3.0f));
    return result;
}

layout(local_size_x = 16, local_size_y = 16) in;
void main() {
    ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
    ivec2 origSize = imageSize(origTex);
    if (storePos.x >= origSize.x || storePos.y >= origSize.y) return;

    vec4 orig = imageLoad(origTex, storePos);
    vec4 cmp = imageLoad(cmpTex, storePos);
    imageStore(resultTex, storePos, vec4(abs(vec3(cmp - orig)), max(1, orig.a + cmp.a)));

    vec3 origRGB = orig.rgb * orig.a;
    vec3 cmpRGB = cmp.rgb * cmp.a;

    vec3 origLab = cieToLAB(rgbToCIE(origRGB));
    vec3 cmpLab = cieToLAB(rgbToCIE(cmpRGB));

    float deltaE = length(cmpLab - origLab);

    imageStore(resultTexDeltaE, storePos, vec4(deltaE, 0.0f, 0.0f, 0.0f));
}
