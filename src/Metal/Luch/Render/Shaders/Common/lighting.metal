#pragma once

#include <metal_stdlib>
#include <metal_matrix>
#include "Common/light.metal"
#include "Common/camera.metal"

using namespace metal;

struct LightingResult
{
    half3 diffuse = 0;
    half3 specular = 0;
};

half D_GGX(half NdotH, half roughness)
{
    half alpha = roughness * roughness;
    half alpha2 = alpha * alpha;
    half den = NdotH * NdotH * (alpha2 - 1) + 1;
    return alpha2 / (M_PI_H * den * den);
}

// TODO this is wrong
half G_CookTorranceGGX(half VdotH, half NdotH, half NdotV, half NdotL)
{
    half intermediate = min(NdotV, NdotL);

    half g = 2 * NdotH * intermediate / (VdotH + 0.00001h);

    return saturate(g);
}

half3 F_Schlick(half cosTheta, half3 F0)
{
    return F0 + (1 - F0) * pow(1 - cosTheta, 5);
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

    half den = 4 * NdotV * NdotL + 0.001;
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
    return max(min(1 - d4/r4, 1.0h), 0.0h) / (d2 + 0.0001h);
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
    half NdotV = saturate(dot(N, V));

    half3 F = F_Schlick(NdotV, F0);
    half3 kD = (1 - metallic);

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

    half NdotV = saturate(dot(N, V));

    half3 F = F_Schlick(NdotV, F0);
    half3 kD = (1 - metallic);
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
    L = L / dist;

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
    L = L / dist;

    LightingResult pointLighting = ApplyPointlLightImpl(light, L, dist, V, P, N, F0, metallic, roughness);
    half spotIntensity = SpotCone(light.innerConeAngle, light.outerConeAngle, directionVS, L);

    result.diffuse = pointLighting.diffuse * spotIntensity;
    result.specular = pointLighting.specular * spotIntensity;

    return result;
}
