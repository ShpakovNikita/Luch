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

struct VertexOut
{
    float4 position [[position]];
    float2 texCoord;
};

struct FragmentOut
{
    half4 luminance [[color(0)]];
};

fragment FragmentOut fp_main(
    VertexOut in [[ stage_in ]],
    constant CameraUniform& camera [[ buffer(0) ]],
    constant LightingParamsUniform& lightingParams [[ buffer(1)] ],
    constant Light* lights [[ buffer(2) ]],
    texture2d<half> gbuffer0 [[ texture(0) ]],
    texture2d<half> gbuffer1 [[ texture(1 )]],
    texture2d<half> gbuffer2 [[ texture(2) ]],
    depth2d<float> depthBuffer [[ texture(3) ]],
    texturecube<half> diffuseIlluminanceCube [[ texture(4) ]],
    texturecube<half> specularReflectionCube [[ texture(5) ]],
    texture2d<half> specularBRDF [[ texture(6) ]])
{
    constexpr sampler gbufferSampler(coord::normalized, filter::nearest);
    constexpr sampler depthBufferSampler(coord::normalized, filter::nearest);

    float2 texCoord = in.texCoord;

    half4 gbuffer0Sample = gbuffer0.sample(gbufferSampler, texCoord);
    half4 gbuffer1Sample = gbuffer1.sample(gbufferSampler, texCoord);

    half3 baseColor = gbuffer0Sample.rgb;
    half3 N = gbuffer1Sample.rgb;

    // Unlit early-out
    if(all(N == 0))
    {
        FragmentOut unlitResult;
        unlitResult.luminance.rgb = baseColor;
        unlitResult.luminance.a = 1;
        return unlitResult;
    }

    half4 gbuffer2Sample = gbuffer2.sample(gbufferSampler, texCoord);
    half depth = depthBuffer.sample(depthBufferSampler, texCoord);

    half occlusion = gbuffer0Sample.a;
    half metallic = gbuffer1Sample.a;
    half3 emittedLuminance = gbuffer2Sample.rgb;
    half linearRoughness = half(gbuffer2Sample.a);

    constexpr half3 dielectricF0 = half3(0.04);
    constexpr half3 black = 0;

    half3 cdiff = mix(baseColor.rgb * (1 - dielectricF0.r), black, metallic);
    half3 F0 = mix(dielectricF0, baseColor.rgb, metallic);

    // Reconstruct view-space position
    half2 attachmentSize = half2(depthBuffer.get_width(), depthBuffer.get_height());
    half2 positionSS = half2(in.position.xy);
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

    FragmentOut result;

    result.luminance.rgb =
        emittedLuminance
        + directLuminance
        + (specularReflectionLuminance + diffuseIndirectLuminance) * occlusion;
    
    result.luminance.a = 1;

    return result;
}
