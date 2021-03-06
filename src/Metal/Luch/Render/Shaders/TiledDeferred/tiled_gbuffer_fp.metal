#include <metal_stdlib>
#include <metal_texture>
#include <simd/simd.h>

#include "Common/material.metal"
#include "Common/utils.metal"

using namespace metal;

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
    half4 gbuffer0 [[color(0)]];
    half4 gbuffer1 [[color(1)]];
    half4 gbuffer2 [[color(2)]];
    float gbufferDepth [[color(3)]];
};

// Figure out coordinate system
#if !ALPHA_MASK
[[early_fragment_tests]]
#endif
fragment FragmentOut fp_main(
    VertexOut in [[stage_in]],
    constant MaterialUniform& material [[buffer(0)]]

#if HAS_BASE_COLOR_TEXTURE
    , texture2d<half> baseColorMap [[texture(0)]]         // RGB - color, A - opacity
    , sampler baseColorSampler [[sampler(0)]]
#endif
#if HAS_METALLIC_ROUGHNESS_TEXTURE
    , texture2d<half> metallicRoughnessMap [[texture(1)]] // B - metallic, G - roughness, RA unused
    , sampler metallicRoughnessSampler [[sampler(1)]]
#endif
#if HAS_NORMAL_TEXTURE
    , texture2d<float> normalMap [[texture(2)]]           // RGB - XYZ, A - unused
    , sampler normalMapSampler [[sampler(2)]]
#endif
#if HAS_OCCLUSION_TEXTURE
    , texture2d<half> occlusionMap [[texture(3)]]         // greyscale,
    , sampler occlusionSampler [[sampler(3)]]
#endif
#if HAS_EMISSIVE_TEXTURE
    , texture2d<half> emissiveMap [[texture(4)]]          // RGB - light color, A unused
    , sampler emissiveSampler [[sampler(4)]]
#endif
    )
{
    FragmentOut out;

    float3 positionVS = in.positionVS;

    #if HAS_TEXCOORD_0
        float2 texCoord = in.texCoord;
    #endif

    half4 baseColor = half4(material.baseColorFactor);

    #if HAS_BASE_COLOR_TEXTURE && HAS_TEXCOORD_0
        half4 baseColorSample = baseColorMap.sample(baseColorSampler, texCoord);
        baseColor *= baseColorSample;
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
        float3 N = normalize(ExtractNormal(normalSample, material.normalScale, TBN));
    #else
        float3 N = normalize(normalVS);
    #endif

    half metallic = half(material.metallicFactor);
    half linearRoughness = half(material.roughnessFactor);

    #if HAS_METALLIC_ROUGHNESS_TEXTURE && HAS_TEXCOORD_0
        half4 metallicRoughnessSample = metallicRoughnessMap.sample(metallicRoughnessSampler, texCoord);
        metallic *= metallicRoughnessSample.b;
        linearRoughness *= clamp(metallicRoughnessSample.g, 0.04h, 1.0h);
    #endif

    half3 emissive = half3(material.emissiveFactor);

    #if HAS_EMISSIVE_TEXTURE && HAS_TEXCOORD_0
        half4 emissiveSample = emissiveMap.sample(emissiveSampler, texCoord);
        emissive *= emissiveSample.rgb;
    #endif

    #if HAS_OCCLUSION_TEXTURE && HAS_TEXCOORD_0
        half occlusion = occlusionMap.sample(occlusionSampler, texCoord).r;
    #else
        half occlusion = 1.0h;
    #endif

    out.gbuffer0.rgb = baseColor.rgb;
    out.gbuffer0.a = mix(1, occlusion, half(material.occlusionStrength));

    out.gbuffer1.rgb = half3(N);
    out.gbuffer1.a = metallic;

    out.gbuffer2.rgb = emissive.rgb;
    out.gbuffer2.a = linearRoughness;

    out.gbufferDepth = in.positionCS.z;

    return out;
}

