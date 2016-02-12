#version 430

uniform sampler2D cocTex;
uniform sampler2D sourceFrontTex;
uniform sampler2D sourceBackTex;
layout(rgba32f) uniform image2D targetTex;

uniform float defocus;
uniform float bloomIntensity;


layout(local_size_x = 32, local_size_y = 16, local_size_z = 1) in;
void main() {
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    ivec2 targetSize = imageSize(targetTex);

    if (pos.x >= targetSize.x || pos.y >= targetSize.y) return;

    vec2 relPos = vec2(pos) / vec2(targetSize);
    vec4 colorCoC = texelFetch(cocTex, pos, 0);
    vec3 color = colorCoC.rgb;
    vec3 back = texture(sourceBackTex, relPos).rgb;
    vec4 front = texture(sourceFrontTex, relPos);

    float normRadius = (colorCoC.a * 2.0f) - 1.0f;

    float a = clamp(1.5f * front.a, 0.0f, 1.0f);
    front.rgb = front.rgb * (a / max(front.a, 0.001f));
    front.a = a;

    if (normRadius > 0.1f) {
        normRadius = min(normRadius * 1.5f, 1.0f);
    }

    vec3 result = mix(color, back, abs(normRadius)) * (1.0f - front.a) + front.rgb;

    imageStore(targetTex, pos, vec4(result, 1.0f));
}
