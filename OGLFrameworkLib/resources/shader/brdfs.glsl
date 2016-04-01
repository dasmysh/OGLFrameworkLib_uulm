
#ifndef BRDF_TYPE
    #define BRDF_TYPE lambert
#endif

#ifndef NUM_MATS
    #define NUM_MATS 1
#endif

#define M_PI 3.1415926535897932384626433832795f

struct Material {
    vec3 diffuseAlbedo;
    float refraction;
    vec3 specularScaling;
    float roughness;
    vec3 forcePadding;
    float specularExponent;
};

layout(std140) uniform materialsBuffer
{
    Material materials[NUM_MATS];
};

float fresnel(float HdotV, float n) {
    float g = sqrt(n*n + HdotV*HdotV - 1.0f);
    float gpc = g + HdotV;
    float gmc = g - HdotV;

    float f1 = gmc / gpc;
    float f2 = (HdotV * gpc - 1.0f) / (HdotV * gmc + 1.0f);
    return f1 * f1 * (1.0f - (f2*f2));
}

float lambertBRDFSpec(vec3 light, vec3 normal, vec3 view, Material mat) {
    return 0.0f;
}

float blinnPhongBRDFSpec(vec3 light, vec3 normal, vec3 view, Material mat) {
    vec3 halfV = normalize(view + light);
    float NdotH = dot(normal, halfV);
    float cosTerm = pow(NdotH, mat.specularExponent);
    return ((mat.specularExponent + 2.0f) / (2.0f * M_PI)) * cosTerm;
}

float torranceSparrowBRDFSpec(vec3 light, vec3 normal, vec3 view, Material mat) {
    vec3 halfV = normalize(view + light);
    float NdotH = dot(normal, halfV);
    float NdotL = dot(normal, light);
    float NdotV = dot(normal, view);
    float HdotV = dot(halfV, view);

    float spec = 0.0f;

    if (NdotL > 0.0f) {
        float g1 = (2.0f * NdotH * NdotV) / HdotV;
        float g2 = (2.0f * NdotH * NdotL) / HdotV;
        float g = min(1.0f, min(g1, g2));
        
        float m = mat.roughness;
        // float dexp = tan(acos(NdotH)) / m;
        float NdotH2 = NdotH * NdotH;
        float NdotH4 = NdotH2 * NdotH2;
        float m2 = m * m;
        float d = (1.0f / (m2*NdotH4)) * exp((NdotH2 - 1.0f) / (m2 * NdotH2));
        float f = fresnel(HdotV, mat.refraction);
        
        spec = (d * g * f) /(NdotL * NdotV * M_PI);
    }
    return spec;
}

#define BRDF_SPEC_PASTE(type, fname, light, normal, view, mat) type ## fname ## (light, normal, view, mat)
#define BRDF_SPEC_PASTE2(type, fname, light, normal, view, mat) BRDF_SPEC_PASTE(type, fname, light, normal, view, mat)
#define brdfSpec(light, normal, view, mat)  BRDF_SPEC_PASTE2(BRDF_TYPE, BRDFSpec, light, normal, view, mat)

vec3 brdf(vec3 light, vec3 normal, vec3 view, Material mat) {
    return (mat.diffuseAlbedo / M_PI) + mat.specularScaling * brdfSpec(light, normal, view, mat);
}

vec3 brdfI(vec3 light, vec3 normal, vec3 view, int matIdx) {
    return brdf(light, normal, view, materials[matIdx]);
}
