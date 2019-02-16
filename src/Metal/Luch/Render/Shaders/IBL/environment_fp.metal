#include <metal_stdlib>
#include <metal_texture>
#include <simd/simd.h>
#include "Common/lighting.metal"
#include "Common/material.metal"

using namespace metal;

constant constexpr int MaxLights = 4;

struct LightsUniform
{
    Light lights[MaxLights];
};

struct LightingParamsUniform
{
    ushort lightCount;
    ushort padding0;
    int padding1;
    int padding2;
    int padding3;
};

struct VertexOut
{
    float4 positionCS [[ position ]];
    ushort faceIndex [[ render_target_array_index ]];

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

float3 ExtractNormal(float3 normalTS, float normalScale, float3x3 TBN)
{
    float3 result = (normalTS * 2 - float3(1.0)) * float3(normalScale, normalScale, 1.0);

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

struct FragmentOut
{
    half4 luminance [[color(0)]];
};

// Figure out coordinate system
#if !ALPHA_MASK
[[early_fragment_tests]]
#endif
fragment FragmentOut fp_main(
    VertexOut in [[stage_in]],
    constant CameraUniform& camera [[buffer(0)]],
    constant MaterialUniform& material [[buffer(1)]],
    constant LightingParamsUniform& lightingParams [[buffer(2)]],
    constant LightsUniform& lights [[buffer(3)]]

#if HAS_BASE_COLOR_TEXTURE
    , texture2d<half> baseColorMap [[texture(0)]]         // RGB - color, A - opacity
    , sampler baseColorSampler [[sampler(0)]]
#endif
#if HAS_METALLIC_ROUGHNESS_TEXTURE
    , texture2d<half> metallicRoughnessMap [[texture(1)]] // R - metallic, G - roughness, BA unused
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
        discard_fragment();
    #endif

    #if HAS_NORMAL && HAS_TANGENT
        float3 tangentVS = normalize(in.tangentVS.xyz);
        float3 bitangentVS = normalize(cross(normalVS, tangentVS)) * in.tangentVS.w;
        float3x3 TBN = float3x3(tangentVS, bitangentVS, normalVS);
    #elif HAS_TEXCOORD_0
        float3x3 TBN = TangentFrame(dp1, dp2, normalVS, texCoord);
        discard_fragment();
    #endif

    #if !HAS_TANGENT && HAS_TEXCOORD_0
        float3 tangentVS = TBN[0];
    #endif

    #if HAS_NORMAL_TEXTURE && HAS_TEXCOORD_0
        float3 normalSample = normalMap.sample(normalMapSampler, texCoord).xyz;
        half3 N = half3(normalize(ExtractNormal(normalSample, material.normalScale, TBN)));
    #else
        half3 N = half3(normalize(normalVS));
        discard_fragment();
    #endif

    half metallic = half(material.metallicFactor);
    half roughness = half(material.roughnessFactor);

    #if HAS_METALLIC_ROUGHNESS_TEXTURE && HAS_TEXCOORD_0
        half4 metallicRoughnessSample = metallicRoughnessMap.sample(metallicRoughnessSampler, texCoord);
        metallic *= metallicRoughnessSample.b;
        roughness *= clamp(metallicRoughnessSample.g, 0.04h, 1.0h);
    #endif

    half3 emitted = half3(material.emissiveFactor);

    #if HAS_EMISSIVE_TEXTURE && HAS_TEXCOORD_0
        half4 emissiveSample = emissiveMap.sample(emissiveSampler, texCoord);
        emitted *= emissiveSample.rgb;
    #endif

    #if HAS_OCCLUSION_TEXTURE && HAS_TEXCOORD_0
        half occlusionSample = occlusionMap.sample(occlusionSampler, texCoord).r;
        half occlusion = mix(1, occlusionSample, half(material.occlusionStrength));
    #else
        half occlusion = 1.0h;
    #endif

    half3 P = half3(in.positionVS.xyz);
    constexpr half3 eyePosVS = half3(0); // in view space eye is at origin
    half3 V = normalize(eyePosVS - P);

    half3 F0 = half3(0.04h);
    // If material is dielectrict, it's reflection coefficient can be approximated by 0.04
    // Otherwise (for metals), take base color to "tint" reflections
    F0 = mix(F0, baseColor.rgb, metallic);

    LightingResult lightingResult;

    for(ushort i = 0; i < lightingParams.lightCount; i++)
    {
        Light light = lights.lights[i];

        LightingResult intermediateResult;

        switch(light.type)
        {
        case LightType::LIGHT_DIRECTIONAL:
            intermediateResult = ApplyDirectionalLight(camera, light, V, N, F0, metallic, roughness);
            break;
        case LightType::LIGHT_POINT:
            intermediateResult = ApplyPointlLight(camera, light, V, P, N, F0, metallic, roughness);
            break;
        case LightType::LIGHT_SPOT:
            intermediateResult = ApplySpotLight(camera, light, V, P, N, F0, metallic, roughness);
            break;
        default:
            intermediateResult = { NAN, NAN };
        }

        lightingResult.diffuse += intermediateResult.diffuse;
        lightingResult.specular += intermediateResult.specular;
    }

    FragmentOut result;

    result.luminance.rgb = emitted + baseColor.rgb * lightingResult.diffuse + lightingResult.specular;
    result.luminance.a = baseColor.a;

    return result;
}

