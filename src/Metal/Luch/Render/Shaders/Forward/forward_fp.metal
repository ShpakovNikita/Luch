#include <metal_stdlib>
#include <metal_texture>
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

constant constexpr int MaxLights = 4;

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

struct LightsUniform
{
    Light lights[MaxLights];
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

struct LightingResult
{
    half3 diffuse = half3(0);
    half3 specular = half3(0);
};

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

