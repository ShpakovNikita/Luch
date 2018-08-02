#version 450
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

#define PI 3.1415926538

layout (std140, set = 0, binding = 0) uniform CameraUniformBufferObject
{
    mat4x4 view;
    mat4x4 projection;
    vec4 positionWS;
    vec2 zMinMax;
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
    float alphaCutoff;
    float metallicFactor;
    float roughnessFactor;
    float normalScale;
    float occlusionStrength;
} material;

layout (location = 0) in vec3 inPositionVS;

#if HAS_NORMAL
    layout (location = 1) in vec3 inNormalVS;
#endif

#if HAS_TANGENT
    layout (location = 2) in vec4 inTangentVS;
#endif

#if HAS_TEXCOORD_0
    layout (location = 3) in vec2 inTexCoord;
#endif

layout (location = 0) out vec4 outBaseColor;
layout (location = 1) out vec4 outNormal;

vec3 ExtractNormal(vec3 normalTS, mat3 TBN)
{
    vec3 result = normalTS * 2 - vec3(1.0);

    return normalize(TBN * normalTS);
}

mat3 TangentFrame(vec3 dp1, vec3 dp2, vec3 N, vec2 uv)
{
    // get edge vectors of the pixel triangle
    vec2 duv1 = dFdx(uv);
    vec2 duv2 = dFdy(uv);
 
    // solve the linear system
    vec3 dp2perp = cross(dp2, N);
    vec3 dp1perp = cross(N, dp1);
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
 
    // construct a scale-invariant frame 
    float invmax = inversesqrt(max(dot(T, T), dot(B,B)));
    return mat3(T * invmax, B * invmax, N);
}

// Figure out coordinate system
void main()
{
    vec3 positionVS = inPositionVS;

    #if !HAS_NORMAL && !HAS_TANGENT
        discard;
    #endif

    #if HAS_TEXCOORD_0
        vec2 texCoord = inTexCoord;
    #endif

    #if HAS_BASE_COLOR_TEXTURE && HAS_TEXCOORD_0
        vec4 baseColor = texture(baseColorMap, texCoord);
    #else
        vec4 baseColor = vec4(1.0, 1.0, 1.0, 1.0);
    #endif

    #if ALPHA_MASK
        if(baseColor.a < material.alphaCutoff)
        {
            discard;
        }
    #endif

    vec3 dp1 = dFdx(positionVS);
    vec3 dp2 = dFdy(positionVS);

    #if HAS_NORMAL
        vec3 normalVS = normalize(inNormalVS);
    #else
        vec3 normalVS = normalize(cross(dp1, dp2));
    #endif

    #if HAS_NORMAL && HAS_TANGENT
        vec3 tangentVS = normalize(inTangentVS.xyz);
        vec3 bitangentVS = normalize(cross(normalVS, tangentVS)) * inTangentVS.w;
        mat3 TBN = mat3(tangentVS, bitangentVS, normalVS);
    #elif HAS_TEXCOORD_0
        mat3 TBN = TangentFrame(dp1, dp2, normalVS, texCoord);
    #endif

    #if !HAS_TANGENT && HAS_TEXCOORD_0
        vec3 tangentVS = TBN[0];
    #endif

    vec3 N;
    #if HAS_NORMAL_TEXTURE && HAS_TEXCOORD_0
        vec3 normalSample = texture(normalMap, texCoord).xyz;
        N = ExtractNormal(normalSample, TBN);
    #else
        N = normalize(normalVS);
    #endif

    #if HAS_METALLIC_ROUGHNESS_TEXTURE && HAS_TEXCOORD_0
        vec4 metallicRoughness = texture(metallicRoughnessMap, texCoord);
        float metallic = material.metallicFactor * metallicRoughness.b;
        float roughness = material.roughnessFactor * clamp(metallicRoughness.g, 0.04, 1.0);
    #else
        float metallic = material.metallicFactor;
        float roughness = material.roughnessFactor;
    #endif

    outBaseColor = baseColor;
    outNormal.xy = N.xy * 0.5 + vec2(0.5);
    outNormal.zw = vec2(metallic, roughness);
}
