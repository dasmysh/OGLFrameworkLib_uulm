
vec3 sphericalToCartesian(vec2 sph) {
    float sint = sin(sph.y);
    return normalize(vec3(sint * cos(sph.x), cos(sph.y), -sint * sin(sph.x)));
}

vec2 cartesianToSpherical(vec3 cart) {
    return vec2(acos(cart.y), atan(-cart.y, cart.x));
}
