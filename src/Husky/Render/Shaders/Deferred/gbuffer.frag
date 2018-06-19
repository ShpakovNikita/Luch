#version 450
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

#define PI 3.1415926538

layout (set = 0, binding = 0) uniform CameraUniformBufferObject
{
    mat4x4 view;
    mat4x4 projection;
    vec3 positionWS;
} camera;

layout (set = 2, binding = 0) uniform sampler2D baseColorMap;         // RGB - color, A - opacity
layout (set = 2, binding = 1) uniform sampler2D metallicRoughnessMap; // R - metallic, G - roughness, BA unused
layout (set = 2, binding = 2) uniform sampler2D normalMap;            // RGB - XYZ, A - unused
layout (set = 2, binding = 3) uniform sampler2D occlusionMap;         // greyscale, 
layout (set = 2, binding = 4) uniform sampler2D emissiveMap;          // RGB - light color, A unused

layout(push_constant)
uniform MaterialPushConstants
{
    vec4 baseColorFactor;
    vec3 emissiveFactor;
    bool hasBaseColorTexture;
    bool hasMetallicRoughnessTexture;
    bool hasNormalTexture;
    bool hasOcclusionTexture;
    bool hasEmissiveTexture;
    bool isAlphaMask;
    float alphaCutoff;
    float metallicFactor;
    float roughnessFactor;
    float normalScale;
    float occlusionStrength;
} material;

layout (location = 0) in vec3 inPositionVS;
layout (location = 1) in vec3 inNormalVS;
#ifdef HAS_TANGENT
layout (location = 2) in vec3 inTangentVS;
#endif
layout (location = 3) in vec2 inTexCoord;

layout (location = 0) out vec4 outBaseColor;
layout (location = 1) out vec4 outNormal;

vec3 ExtractNormal(vec3 normalTS, vec3 T, vec3 N)
{
    vec3 result = normalTS * 2 - vec3(1.0);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * normalTS);
}

void main()
{
    vec3 positionVS = inPositionVS;
    vec3 normalVS = inNormalVS;
#ifdef HAS_TANGENT
    vec3 tangentVS = inTangentVS;
#else
    vec3 q1 = dFdx(inPositionVS);
    vec3 q2 = dFdy(inPositionVS);
    vec2 st1 = dFdx(inTexCoord);
    vec2 st2 = dFdy(inTexCoord);
    vec3 tangentVS = normalize(q1 * st2.t - q2 * st1.t);
#endif
    vec2 texCoord = inTexCoord;

    // TODO normal mapping
    vec3 N;
    if(material.hasNormalTexture)
    {
        vec3 normalTS = texture(normalMap, texCoord).xyz;
        N = ExtractNormal(normalTS, tangentVS, normalVS);
    }
    else
    {
        N = normalize(normalVS);
    }

    // Test code
    N = normalize(normalVS);

    vec4 baseColor = texture(baseColorMap, texCoord);

    float metallic = material.metallicFactor;
    float roughness = material.roughnessFactor;

    if(material.hasMetallicRoughnessTexture)
    {
        vec4 metallicRoughness = texture(metallicRoughnessMap, texCoord);
        metallic *= metallicRoughness.b;
        roughness *= clamp(metallicRoughness.g, 0.04, 1.0);
    }

    outBaseColor = baseColor;
    outNormal.xyz = N;
    outNormal.a = 1.0;
}
