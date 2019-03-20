#include <metal_stdlib>
#include <simd/simd.h>
#include "Common/lighting.metal"
#include "Common/camera.metal"
#include "IBL/ibl_lighting.metal"

using namespace metal;

struct LightingParamsUniform
{
    ushort lightCount;
    ushort padding0;
    int padding1;
    int padding2;
    int padding3;
};

half2 FragCoordToNDC(half2 fragCoord, half2 size)
{
    half2 pd = 2 * fragCoord / size - half2(1);
    return half2(pd.x, -pd.y);
}

struct ImageBlock
{
    half4 gbuffer0 [[color(0)]];
    half4 gbuffer1 [[color(1)]];
    half4 gbuffer2 [[color(2)]];
    float gbufferDepth [[color(3)]];
    half4 luminance [[color(4)]];
};

kernel void tile_main(
    imageblock<ImageBlock,imageblock_layout_implicit> imageBlock,
    ushort2 gridSize [[threads_per_grid]],
    ushort2 gid [[ thread_position_in_grid ]],
    ushort2 lid [[ thread_position_in_threadgroup ]],
    constant CameraUniform& camera [[ buffer(0) ]],
    constant LightingParamsUniform& lightingParams [[ buffer(1) ]],
    constant Light* lights [[ buffer(2) ]],
    texturecube<half> diffuseIlluminanceCube [[ texture(0) ]],
    texturecube<half> specularReflectionCube [[ texture(1) ]],
    texture2d<half> specularBRDF [[ texture(2) ]])
{
    ImageBlock img = imageBlock.read(lid);

    half3 baseColor = img.gbuffer0.rgb;
    half occlusion = img.gbuffer0.a;

    half3 N = img.gbuffer1.rgb;

    // Unlit early-out
    if(all(N == 0))
    {
        img.luminance.rgb = baseColor;
        img.luminance.a = 1;
        imageBlock.write(img, lid);
        return;
    }

    half metallic = half(img.gbuffer1.a);
    half linearRoughness = half(img.gbuffer2.a);

    half3 emittedLuminance = img.gbuffer2.rgb;

    float depth = img.gbufferDepth;

    constexpr half3 dielectricF0 = half3(0.04);
    constexpr half3 black = 0;

    half3 cdiff = mix(baseColor.rgb * (1 - dielectricF0.r), black, metallic);
    half3 F0 = mix(dielectricF0, baseColor.rgb, metallic);

    // Reconstruct view-space position
    half2 attachmentSize = half2(gridSize);
    half2 positionSS = half2(gid);
    half2 xyNDC = FragCoordToNDC(positionSS, attachmentSize);
    float4 intermediatePosition = camera.inverseProjection * float4(xyNDC.x, xyNDC.y, depth, 1);
    half3 P = half3(intermediatePosition.xyz / intermediatePosition.w);
    constexpr half3 eyePosVS = half3(0); // in view space eye is at origin
    half3 V = normalize(eyePosVS - P);

    half3 directLuminance = 0.0;

    for(ushort i = 0; i < lightingParams.lightCount; i++)
    {
        Light light = lights[i];

        half3 intermediateLuminance;

        switch(light.type)
        {
        case LightType::LIGHT_DIRECTIONAL:
            intermediateLuminance = ApplyDirectionalLight(camera, light, V, N, F0, cdiff, metallic, linearRoughness);
            break;
        case LightType::LIGHT_POINT:
            intermediateLuminance = ApplyPointLight(camera, light, V, P, N, F0, cdiff, metallic, linearRoughness);
            break;
        case LightType::LIGHT_SPOT:
            intermediateLuminance = ApplySpotLight(camera, light, V, P, N, F0, cdiff, metallic, linearRoughness);
            break;
        default:
            intermediateLuminance = { NAN, NAN };
        }

        directLuminance += intermediateLuminance;
    }

    float3 R = float3(reflect(-V, N));
    half NdotV = clamp(abs(dot(N, V)), 0.00001h, 1.0h);

    // TODO think about non-uniform scale
    float3 reflectedWS = (camera.inverseView * float4(R, 0.0)).xyz;

    half3 diffuseIndirectLuminance = CalculateIndirectDiffuse(
        diffuseIlluminanceCube,
        reflectedWS,
        cdiff);

    half3 specularReflectionLuminance = CalculateSpecularReflection(
        specularReflectionCube,
        specularBRDF,
        F0,
        reflectedWS,
        NdotV,
        linearRoughness);

    half4 resultLuminance;

    resultLuminance.rgb =
        emittedLuminance
        + directLuminance
        + (specularReflectionLuminance + diffuseIndirectLuminance) * occlusion;

    resultLuminance.a = 1;

    img.luminance = resultLuminance;

    imageBlock.write(img, lid);
}
