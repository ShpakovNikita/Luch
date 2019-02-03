#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

enum class LightType : ushort
{
    LIGHT_POINT = 0,
    LIGHT_SPOT = 1,
    LIGHT_DIRECTIONAL = 2,
};

enum class LightState : ushort
{
    LIGHT_DISABLED = 0,
    LIGHT_ENABLED = 1,
};

struct Light
{
    float4 positionWS;
    float4 directionWS;
    float4 color;
    LightState state;
    LightType type;
    float innerConeAngle;
    float outerConeAngle;
    float range;
    float intensity;
    float padding0;
    float padding1;
    float padding3;
};

struct LightingResult
{
    half3 diffuse = half3(0);
    half3 specular = half3(0);
};

struct CameraUniform
{
    float4x4 view;
    float4x4 inverseView;
    float4x4 projection;
    float4x4 inverseProjection;
    float4x4 viewProjection;
    float4x4 inverseViewProjection;
    float4 positionWS;
};

struct LightingParamsUniform
{
    ushort lightCount;
    ushort padding0;
    int padding1;
    int padding2;
    int padding3;
};

half D_GGX(half NdotH, half roughness)
{
    half alpha = roughness * roughness;
    half alpha2 = alpha * alpha;
    half den = NdotH * NdotH * (alpha2 - 1.0h) + 1.0h;
    return alpha2 / (M_PI_H * den * den);
}

half G_CookTorranceGGX(half VdotH, half NdotH, half NdotV, half NdotL)
{
    half intermediate = min(NdotV, NdotL);

    half g = 2 * NdotH * intermediate / (VdotH + 0.00001h);

    return saturate(g);
}

half3 F_Schlick(half cosTheta, half3 F0)
{
    return F0 + (1.0h - F0) * pow(1.0h - cosTheta, 5.0h);
}

half3 DiffuseLighting(half3 color, half3 L, half3 N)
{
    half NdotL = max(half(dot(N, L)), 0.0h);
    return color * NdotL;
}

half3 SpecularLighting(half3 color, half3 V, half3 L, half3 N, half3 F, half roughness)
{
    half3 H = normalize(V + L);

    half VdotH = saturate(dot(V, H));
    half NdotV = saturate(dot(N, V));
    half NdotH = saturate(dot(N, H));
    half NdotL = saturate(dot(N, L));

    half den = 4 * NdotV * NdotL + 0.001h;
    half D = D_GGX(NdotH, roughness);
    half G = G_CookTorranceGGX(VdotH, NdotH, NdotV, NdotL);
    half3 spec = D * F * G / den;

    return color * spec;
}

half Attenuation(Light light, half d)
{
    half d2 = d * d;
    half d4 = d2 * d2;
    half r2 = half(light.range) * half(light.range);
    half r4 = r2 * r2;
    return max(min(1.0h - d4/r4, 1.0h), 0.0h) / (d2 + 0.0001h);
}

LightingResult ApplyDirectionalLight(
    CameraUniform camera,
    Light light,
    half3 V,
    half3 N,
    half3 F0,
    half metallic,
    half roughness)
{
    LightingResult result;

    half3 color = half3(light.color.xyz);
    half3 directionVS = half3((camera.view * light.directionWS).xyz);
    half3 L = directionVS.xyz;
    half NdotV = saturate(half(dot(N, V)));

    half3 F = F_Schlick(NdotV, F0);
    half3 kD = (1 - metallic)*(half3(1.0h) - F);

    result.diffuse = kD * DiffuseLighting(color, L, N) * light.intensity;
    result.specular = SpecularLighting(color, V, L, N, F, roughness) * light.intensity;

    return result;
}

LightingResult ApplyPointlLightImpl(
    Light light,
    half3 L,
    half dist,
    half3 V,
    half3 P,
    half3 N,
    half3 F0,
    half metallic,
    half roughness)
{
    LightingResult result;

    half3 color = half3(light.color.xyz);
    half attenuation = Attenuation(light, dist);

    half NdotV = saturate(half(dot(N, V)));

    half3 F = F_Schlick(NdotV, F0);
    half3 kD = (1 - metallic)*(half3(1.0h) - F);
    half intensity = half(light.intensity) * attenuation;

    result.diffuse = kD * DiffuseLighting(color, L, N) * intensity;
    result.specular = SpecularLighting(color, V, L, N, F, roughness) * intensity;

    return result;
}

LightingResult ApplyPointlLight(
    CameraUniform camera,
    Light light,
    half3 V,
    half3 P,
    half3 N,
    half3 F0,
    half metallic,
    half roughness)
{
    half3 positionVS = half3((camera.view * light.positionWS).xyz);
    half3 L = positionVS.xyz - P;
    half dist = length(L);
    L = L/dist;

    return ApplyPointlLightImpl(light, L, dist, V, P, N, F0, metallic, roughness);
}

half SpotCone(half innerConeAngle, half outerConeAngle, half3 directionVS, half3 L)
{
    half lightAngleScale = 1.0h / max(0.001h, cos(innerConeAngle) - cos(outerConeAngle));
    half lightAngleOffset = -cos(outerConeAngle) * lightAngleScale;
    half cosAngle = dot(directionVS, -L);
    half angularAttenuation = saturate(cosAngle * lightAngleScale + lightAngleOffset);
    return angularAttenuation * angularAttenuation;
}

LightingResult ApplySpotLight(
    CameraUniform camera,
    Light light,
    half3 V,
    half3 P,
    half3 N,
    half3 F0,
    half metallic,
    half roughness)
{
    LightingResult result;

    half3 positionVS = half3((camera.view * light.positionWS).xyz);
    half3 directionVS = half3((camera.view * light.directionWS).xyz);
    half3 L = positionVS - P;
    half dist = length(L);
    L = L/dist;

    LightingResult pointLighting = ApplyPointlLightImpl(light, L, dist, V, P, N, F0, metallic, roughness);
    half spotIntensity = SpotCone(light.innerConeAngle, light.outerConeAngle, directionVS, L);

    result.diffuse = pointLighting.diffuse * spotIntensity;
    result.specular = pointLighting.specular * spotIntensity;

    return result;
}

half2 FragCoordToNDC(half2 fragCoord, half2 size)
{
    half2 pd = 2 * fragCoord / size - half2(1.0h);
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
    constant CameraUniform& camera [[buffer(0)]],
    constant LightingParamsUniform& lightingParams [[buffer(1)]],
    constant Light* lights [[buffer(2)]])
{
    ImageBlock img = imageBlock.read(lid);

    half3 baseColor = img.gbuffer0.rgb;
    half occlusion = img.gbuffer0.a;

    half3 N = img.gbuffer1.rgb;
    half metallic = half(img.gbuffer1.a);
    half roughness = half(img.gbuffer2.a);

    half3 emitted = img.gbuffer2.rgb;

    float depth = img.gbufferDepth;

    half3 F0 = half3(0.04h);
    // If material is dielectrict, it's reflection coefficient can be approximated by 0.04
    // Otherwise (for metals), take base color to "tint" reflections
    F0 = mix(F0, baseColor, metallic);

    // Reconstruct view-space position
    half2 attachmentSize = half2(gridSize);
    half2 positionSS = half2(gid);
    half2 xyNDC = FragCoordToNDC(positionSS, attachmentSize);
    float4 intermediatePosition = camera.inverseProjection * float4(xyNDC.x, xyNDC.y, depth, 1.0);
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

    img.luminance.rgb = emitted + baseColor * lightingResult.diffuse + lightingResult.specular;
    img.luminance.a = 1.0;

    imageBlock.write(img, lid);
}
