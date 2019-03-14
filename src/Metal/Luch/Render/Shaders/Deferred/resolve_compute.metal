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

kernel void kernel_main(
    ushort2 gid [[thread_position_in_grid]],
    constant CameraUniform& camera [[ buffer(0) ]],
    constant LightingParamsUniform& lightingParams [[ buffer(1) ]],
    constant Light* lights [[ buffer(2) ]],
    texture2d<half, access::read> gbuffer0 [[ texture(0) ]],
    texture2d<half, access::read> gbuffer1 [[ texture(1) ]],
    texture2d<half, access::read> gbuffer2 [[ texture(2) ]],
    depth2d<float, access::read> depthBuffer [[ texture(3) ]],
    texture2d<half, access::write> luminance [[ texture(4) ]],
    texturecube<half> diffuseIlluminanceCube [[ texture(5) ]],
    texturecube<half> specularReflectionCube [[ texture(6) ]],
    texture2d<half> specularBRDF [[ texture(7) ]])
{
    half4 gbuffer0Sample = gbuffer0.read(gid);
    half4 gbuffer1Sample = gbuffer1.read(gid);
    half4 gbuffer2Sample = gbuffer2.read(gid);
    half depth = depthBuffer.read(gid);

    half3 baseColor = gbuffer0Sample.rgb;
    half occlusion = gbuffer0Sample.a;

    half3 N = gbuffer1Sample.rgb;
    half metallic = half(gbuffer1Sample.a);
    half linearRoughness = half(gbuffer2Sample.a);

    half3 emittedLuminance = gbuffer2Sample.rgb;

    constexpr half3 dielectricF0 = half3(0.04);
    constexpr half3 black = 0;

    half3 cdiff = mix(baseColor.rgb * (1 - dielectricF0.r), black, metallic);
    half3 F0 = mix(dielectricF0, baseColor.rgb, metallic);

    // Reconstruct view-space position
    half2 attachmentSize = half2(depthBuffer.get_width(), depthBuffer.get_height());
    half2 positionSS = half2(gid);
    half2 xyNDC = FragCoordToNDC(positionSS, attachmentSize);
    float4 intermediatePosition = camera.inverseProjection * float4(xyNDC.x, xyNDC.y, depth, 1);
    half3 P = half3(intermediatePosition.xyz / intermediatePosition.w);
    constexpr half3 eyePosVS = 0; // in view space eye is at origin
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

    half3 resultLuminance =
        emittedLuminance
        + directLuminance
        + (specularReflectionLuminance + diffuseIndirectLuminance) * occlusion;

    luminance.write(half4(resultLuminance, 1.0), gid);
}
