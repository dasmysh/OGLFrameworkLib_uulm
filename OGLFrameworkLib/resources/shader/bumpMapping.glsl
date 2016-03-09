
vec3 bumpMapNormal(in sampler2D bmtex, in vec2 tc, in vec3 normal, in vec3 tangent, in vec3 binormal, in float bm) {
    const vec2 size = vec2(2.0, 0.0);
    const ivec3 off = ivec3(-1, 0, 1);

    float s11 = bm * texture(bmtex, tc).x;
    float s01 = bm * textureOffset(bmtex, tc, off.xy).x;
    float s21 = bm * textureOffset(bmtex, tc, off.zy).x;
    float s10 = bm * textureOffset(bmtex, tc, off.yx).x;
    float s12 = bm * textureOffset(bmtex, tc, off.yz).x;
    vec3 va = normalize(vec3(size.xy, s21 - s11));
    vec3 vb = normalize(vec3(size.yx, s12 - s11));
    vec3 bump = normalize(cross(va, vb));

    mat3 tbn = mat3(tangent, binormal, normal);
    return tbn * bump.xyz;
}
