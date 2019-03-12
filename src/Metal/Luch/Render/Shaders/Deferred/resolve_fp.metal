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
    texturecube<half> diffuseIrradianceMap [[ texture(4) ]],
    texturecube<half> specularReflectionMap [[ texture(5) ]],
    texture2d<half> specularBRDF [[ texture(6) ]])
{
    constexpr sampler gbufferSampler(coord::normalized, filter::nearest);
    constexpr sampler depthBufferSampler(coord::normalized, filter::nearest);

    float2 texCoord = in.texCoord;

    half4 gbuffer0Sample = gbuffer0.sample(gbufferSampler, texCoord);
    half4 gbuffer1Sample = gbuffer1.sample(gbufferSampler, texCoord);
    half4 gbuffer2Sample = gbuffer2.sample(gbufferSampler, texCoord);
    half depth = depthBuffer.sample(depthBufferSampler, texCoord);

    half3 baseColor = gbuffer0Sample.rgb;
    half occlusion = gbuffer0Sample.a;

    half3 N = gbuffer1Sample.rgb;
    half metallic = half(gbuffer1Sample.a);
    half roughness = half(gbuffer2Sample.a);

    half3 emitted = gbuffer2Sample.rgb;

    half3 F0 = half3(0.04);
    // If material is dielectrict, it's reflection coefficient can be approximated by 0.04
    // Otherwise (for metals), take base color to "tint" reflections
    F0 = mix(F0, baseColor, metallic);

    // Reconstruct view-space position
    half2 attachmentSize = half2(depthBuffer.get_width(), depthBuffer.get_height());
    half2 positionSS = half2(in.position.xy);
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

    FragmentOut result;

    half3 diffuseDirect = baseColor.rgb * lightingResult.diffuse;
    half3 specularDirect = lightingResult.specular;

    result.luminance.rgb =
        emitted
        + diffuseDirect
        + specularDirect
        + (specularReflectionLuminance + diffuseIndirectLuminance) * occlusion;
    
    result.luminance.a = 1;

    return result;
}
