// shadertype=glsl

vec3 rgb2hsv(vec3 rgb)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = rgb.g < rgb.b ? vec4(rgb.b, rgb.g, K.w, K.z) : vec4(rgb.g, rgb.b, K.x, K.y);
    vec4 q = rgb.r < p.x ? vec4(p.x, p.y, p.w, rgb.r) : vec4(rgb.r, p.y, p.z, p.x);

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 hsv)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 cx3 = vec3(hsv.x, hsv.x, hsv.x);
    vec3 Kw3 = vec3(K.w, K.w, K.w);
    vec3 Kx3 = vec3(K.x, K.x, K.x);
    vec3 p = abs(fract(cx3 + vec3(K)) * 6.0f - Kw3);
    return vec3(hsv.z * mix(Kx3, clamp(p - Kx3, 0.0f, 1.0f), hsv.y));
}
