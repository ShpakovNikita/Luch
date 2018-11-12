#include <metal_stdlib>
#include <metal_texture>
#include <simd/simd.h>

using namespace metal;

struct MaterialUniform
{
    packed_float4 baseColorFactor;
    packed_float3 emissiveFactor;
    float alphaCutoff;
    float metallicFactor;
    float roughnessFactor;
    float normalScale;
    float occlusionStrength;
};

struct VertexOut
{
    float4 positionCS [[position]];
    float3 positionVS;

    #if HAS_NORMAL
        float3 normalVS;
    #endif

    #if HAS_TANGENT
        float4 tangentVS;
    #endif

    #if HAS_TEXCOORD_0
        float2 texCoord;
    #endif
};

struct FragmentOut
{
    float4 baseColor [[color(0)]];
    float4 normal [[color(1)]];
};

float3 ExtractNormal(float3 normalTS, float3x3 TBN)
{
    float3 result = normalTS * 2 - float3(1.0);

    return normalize(TBN * result);
}

float3x3 TangentFrame(float3 dp1, float3 dp2, float3 N, float2 uv)
{
    // get edge vectors of the pixel triangle
    float2 duv1 = dfdx(uv);
    float2 duv2 = dfdy(uv);

    // solve the linear system
    float3 dp2perp = cross(dp2, N);
    float3 dp1perp = cross(N, dp1);
    float3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    float3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    // construct a scale-invariant frame
    float invmax = rsqrt(max(dot(T, T), dot(B,B)));
    return float3x3(T * invmax, B * invmax, N);
}

// Figure out coordinate system
#if !ALPHA_MASK
[[early_fragment_tests]]
#endif
fragment FragmentOut fp_main(
    VertexOut in [[stage_in]],
    device MaterialUniform& material [[buffer(0)]]

#if HAS_BASE_COLOR_TEXTURE
    , texture2d<float> baseColorMap [[texture(0)]]         // RGB - color, A - opacity
    , sampler baseColorSampler [[sampler(0)]]
#endif
#if HAS_METALLIC_ROUGHNESS_TEXTURE
    , texture2d<float> metallicRoughnessMap [[texture(1)]] // R - metallic, G - roughness, BA unused
    , sampler metallicRoughnessSampler [[sampler(1)]]
#endif
#if HAS_NORMAL_TEXTURE
    , texture2d<float> normalMap [[texture(2)]]            // RGB - XYZ, A - unused
    , sampler normalMapSampler [[sampler(2)]]
#endif
#if HAS_OCCLUSION_TEXTURE
    , texture2d<float> occlusionMap [[texture(3)]]         // greyscale,
    , sampler occlusionSampler [[sampler(3)]]
#endif
#if HAS_EMISSIVE_TEXTURE
    , texture2d<float> emissiveMap [[texture(4)]]          // RGB - light color, A unused
    , sampler emissiveSampler [[sampler(4)]]
#endif
    )
{
    FragmentOut out;

    float3 positionVS = in.positionVS;

    #if HAS_TEXCOORD_0
        float2 texCoord = in.texCoord;
    #endif

    #if HAS_BASE_COLOR_TEXTURE && HAS_TEXCOORD_0
        float4 baseColor = baseColorMap.sample(baseColorSampler, texCoord);
    #else
        float4 baseColor = float4(1.0);
    #endif

    #if ALPHA_MASK
        if(baseColor.a < material.alphaCutoff)
        {
            discard_fragment();
        }
    #endif

    float3 dp1 = dfdx(positionVS);
    float3 dp2 = dfdy(positionVS);

    #if HAS_NORMAL
        float3 normalVS = normalize(in.normalVS);
    #else
        float3 normalVS = normalize(cross(dp1, dp2));
    #endif

    #if HAS_NORMAL && HAS_TANGENT
        float3 tangentVS = normalize(in.tangentVS.xyz);
        float3 bitangentVS = normalize(cross(normalVS, tangentVS)) * in.tangentVS.w;
        float3x3 TBN = float3x3(tangentVS, bitangentVS, normalVS);
    #elif HAS_TEXCOORD_0
        float3x3 TBN = TangentFrame(dp1, dp2, normalVS, texCoord);
    #endif

    #if !HAS_TANGENT && HAS_TEXCOORD_0
        float3 tangentVS = TBN[0];
    #endif

    #if HAS_NORMAL_TEXTURE && HAS_TEXCOORD_0
        float3 normalSample = normalMap.sample(normalMapSampler, texCoord).xyz;
        float3 N = normalize(ExtractNormal(normalSample, TBN));
    #else
        float3 N = normalize(normalVS);
    #endif

    #if HAS_METALLIC_ROUGHNESS_TEXTURE && HAS_TEXCOORD_0
        float4 metallicRoughness = metallicRoughnessMap.sample(metallicRoughnessSampler, texCoord);
        float metallic = material.metallicFactor * metallicRoughness.b;
        float roughness = material.roughnessFactor * clamp(metallicRoughness.g, 0.04, 1.0);
    #else
        float metallic = material.metallicFactor;
        float roughness = material.roughnessFactor;
    #endif

    out.baseColor = baseColor;

    out.normal.xy = N.xy;
    out.normal.zw = float2(metallic, roughness);

    return out;
}

