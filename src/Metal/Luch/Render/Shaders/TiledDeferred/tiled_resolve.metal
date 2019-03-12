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
    texturecube<half> diffuseIrradianceMap [[ texture(0) ]],
    texturecube<half> specularReflectionMap [[ texture(1) ]],
    texture2d<half> specularBRDF [[ texture(2) ]])
{
    ImageBlock img = imageBlock.read(lid);

    half3 baseColor = img.gbuffer0.rgb;
    half occlusion = img.gbuffer0.a;

    half3 N = img.gbuffer1.rgb;
    half metallic = half(img.gbuffer1.a);
    half roughness = half(img.gbuffer2.a);

    half3 emitted = img.gbuffer2.rgb;

    float depth = img.gbufferDepth;

    half3 F0 = half3(0.04);
    // If material is dielectrict, it's reflection coefficient can be approximated by 0.04
    // Otherwise (for metals), take base color to "tint" reflections
    F0 = mix(F0, baseColor, metallic);

    // Reconstruct view-space position
    half2 attachmentSize = half2(gridSize);
    half2 positionSS = half2(gid);
    half2 xyNDC = FragCoordToNDC(positionSS, attachmentSize);
    float4 intermediatePosition = camera.inverseProjection * float4(xyNDC.x, xyNDC.y, depth, 1);
    half3 P = half3(intermediatePosition.xyz / intermediatePosition.w);
    constexpr half3 eyePosVS = half3(0); // in view space eye is at origin
    half3 V = normalize(eyePosVS - P);

    LightingResult lightingResult;

    for(ushort i = 0; i < lightingParams.lightCount; i++)
    {
        Light light = lights[i];

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

    float3 R = float3(reflect(-V, N));
    half NdotV = half(saturate(dot(N, V)));

    // TODO think about non-uniform scale
    float3 reflectedWS = (camera.inverseView * float4(R, 0)).xyz;

    half3 diffuseIndirectLuminance = CalculateIndirectDiffuse(
        diffuseIrradianceMap,
        reflectedWS,
        baseColor.rgb,
        metallic);

    half3 specularReflectionLuminance = CalculateSpecularReflection(
        specularReflectionMap,
        specularBRDF,
        F0,
        reflectedWS,
        NdotV,
        metallic,
        roughness);

    half3 diffuseDirect = baseColor.rgb * lightingResult.diffuse;
    half3 specularDirect = lightingResult.specular;

    half4 resultLuminance;

    resultLuminance.rgb =
        emitted
        + diffuseDirect
        + specularDirect
        + (specularReflectionLuminance + diffuseIndirectLuminance) * occlusion;

    resultLuminance.a = 1;

    img.luminance = resultLuminance;

    imageBlock.write(img, lid);
}
