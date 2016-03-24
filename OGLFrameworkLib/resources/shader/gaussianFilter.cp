#version 430

uniform sampler2D sourceTex;
layout(TEX_FORMAT) uniform image2D targetTex;
uniform vec2 dir;
uniform float bloomWidth;

// blur code from https://software.intel.com/en-us/blogs/2014/07/15/an-investigation-of-fast-real-time-gpu-based-image-blur-algorithms
vec3 GaussianBlur(sampler2D tex0, vec2 centreUV, vec2 pixelOffset) {
    vec3 colOut = vec3(0, 0, 0);
    ////////////////////////////////////////////////;
    // Kernel width 7 x 7
    //
    const int stepCount = 2;
    //
    const float gWeights[stepCount] ={
       0.44908,
       0.05092
    };
    const float gOffsets[stepCount] ={
       0.53805,
       2.06278
    };
    ////////////////////////////////////////////////;

    for(int i = 0; i < stepCount; i++) {
        vec2 texCoordOffset = gOffsets[i] * pixelOffset;
        vec3 color = texture(tex0, centreUV + texCoordOffset).xyz;
        color += texture(tex0, centreUV - texCoordOffset).xyz;
        colOut += gWeights[i] * color;
    }

    return colOut;
} 

layout(local_size_x = 32, local_size_y = 16, local_size_z = 1) in;
void main() {
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    ivec2 targetSize = imageSize(targetTex);

    if (pos.x >= targetSize.x || pos.y >= targetSize.y) return;

    ivec2 sourceSize = textureSize(sourceTex, 0);
    vec2 dirPixels = (dir / vec2(sourceSize)) * bloomWidth;
    vec2 tex = (vec2(pos) + vec2(0.5f)) / vec2(targetSize);

    vec4 result = vec4(GaussianBlur(sourceTex, tex, dirPixels), 1.0f);
    imageStore(targetTex, pos, result);
}
