#pragma once

#include <metal_stdlib>
#include <metal_matrix>
#include "Common/light.metal"
#include "Common/camera.metal"

using namespace metal;

// roughness is not the same as linear roughness
// roughness is "alpha"

half D_GGX(half NdotH, half a2)
{
    half den = (NdotH * a2 - NdotH) * NdotH + 1;
    return a2 / (M_PI_H * den * den) ;
}

half V_SmithGGXCorrelated(half NdotL, half NdotV, half a2)
{
    half Lambda_GGXV = NdotL * sqrt((-NdotV * a2 + NdotV) * NdotV + a2);
    half Lambda_GGXL = NdotV * sqrt((-NdotL * a2 + NdotL) * NdotL + a2);
    return 0.5f / (Lambda_GGXV + Lambda_GGXL);
}

half G_SmithGGXCorrelated(half NdotL, half NdotV, half a2)
{
    half NdotL2 = NdotL * NdotL;
    half NdotV2 = NdotV * NdotV;
    half lambdaV = (-1 + sqrt(a2 * (1 - NdotL2) / NdotL2 + 1)) * 0.5;
    half lambdaL = (-1 + sqrt(a2 * (1 - NdotV2) / NdotV2 + 1)) * 0.5;
    return 1 / (1 + lambdaV + lambdaL);
}

half3 F_Schlick(half3 F0, half F90, half cosTheta)
{
    return F0 + (half3(F90) - F0) * pow(1 - cosTheta, 5);
}

half3 F_Schlick(half3 F0, half cosTheta)
{
    return F_Schlick(F0, 1.0, cosTheta);
}

half Fr_DisneyDiffuse(half NdotV, half NdotL, half LdotH, half linearRoughness)
{
    half energyBias = mix(0.0h, 0.5h, linearRoughness);
    half energyFactor = mix(1.0h, 1.0h / 1.51h, linearRoughness);
    half Fd90 = energyBias + 2 * LdotH * LdotH * linearRoughness;
    half3 F0 = half3(1);
    half lightScatter = F_Schlick(F0, Fd90, NdotL).r;
    half viewScatter = F_Schlick(F0, Fd90, NdotV).r;

    return lightScatter * viewScatter * energyFactor / M_PI_H;
}

half Fr_Lambert()
{
    return 1 / M_PI_H;
}

half3 Specular(half3 F0, half NdotH, half NdotL, half NdotV, half LdotH, half linearRoughness)
{
    half a = linearRoughness * linearRoughness;
    half a2 = a * a;
    half D = D_GGX(NdotH, a2);
    half V = V_SmithGGXCorrelated(NdotL, NdotV, a2);
    half3 F = F_Schlick(F0, LdotH);

    return (D * V) * F;
}

half3 Luminance(
    half3 F0,
    half3 cdiff,
    half3 V,
    half3 L,
    half3 N,
    half metallic,
    half linearRoughness)
{
    half3 H = normalize(V + L);

    half NdotV = clamp(abs(dot(N, V)),  0.00001h, 1.0h);
    half LdotH = saturate(dot(L, H));
    half NdotH = saturate(dot(N, H));
    half NdotL = saturate(dot(N, L));

    //half Fd = Fr_DisneyDiffuse(NdotV, NdotL, LdotH, linearRoughness);
    half Fd = Fr_Lambert();
    half3 specular = Specular(F0, NdotH, NdotL, NdotV, LdotH, linearRoughness);

    return (Fd * cdiff + specular) * NdotL;
}

half Attenuation(Light light, half d)
{
    half d2 = d * d;
    half d4 = d2 * d2;
    half r2 = half(light.range) * half(light.range);
    half r4 = r2 * r2;
    return max(min(1 - d4/r4, 1.0h), 0.0h) / (d2 + 0.0001h);
}

half3 ApplyDirectionalLight(
    CameraUniform camera,
    Light light,
    half3 V,
    half3 N,
    half3 F0,
    half3 cdiff,
    half metallic,
    half linearRoughness)
{
    half3 lightColor = half3(light.color.rgb);
    half3 directionVS = half3((camera.view * light.directionWS).xyz);
    half3 L = directionVS.xyz;

    half3 luminance = Luminance(F0, cdiff, V, L, N, metallic, linearRoughness);

    return luminance * lightColor * light.intensity;
}

half3 ApplyPointLightImpl(
    Light light,
    half3 L,
    half dist,
    half3 V,
    half3 P,
    half3 N,
    half3 F0,
    half3 cdiff,
    half metallic,
    half linearRoughness)
{
    half3 lightColor = half3(light.color.xyz);
    half attenuation = Attenuation(light, dist);

    half intensity = half(light.intensity) * attenuation;

    half3 luminance = Luminance(F0, cdiff, V, L, N, metallic, linearRoughness);

    return luminance * lightColor * intensity;
}

half3 ApplyPointLight(
    CameraUniform camera,
    Light light,
    half3 V,
    half3 P,
    half3 N,
    half3 F0,
    half3 cdiff,
    half metallic,
    half linearRoughness)
{
    half3 positionVS = half3((camera.view * light.positionWS).xyz);
    half3 L = positionVS.xyz - P;
    half dist = length(L);
    L = L / dist;

    return ApplyPointLightImpl(light, L, dist, V, P, N, F0, cdiff, metallic, linearRoughness);
}

half SpotCone(half innerConeAngle, half outerConeAngle, half3 directionVS, half3 L)
{
    half lightAngleScale = 1.0h / max(0.001h, cos(innerConeAngle) - cos(outerConeAngle));
    half lightAngleOffset = -cos(outerConeAngle) * lightAngleScale;
    half cosAngle = dot(directionVS, -L);
    half angularAttenuation = saturate(cosAngle * lightAngleScale + lightAngleOffset);
    return angularAttenuation * angularAttenuation;
}

half3 ApplySpotLight(
    CameraUniform camera,
    Light light,
    half3 V,
    half3 P,
    half3 N,
    half3 F0,
    half3 cdiff,
    half metallic,
    half linearRoughness)
{
    half3 positionVS = half3((camera.view * light.positionWS).xyz);
    half3 directionVS = half3((camera.view * light.directionWS).xyz);
    half3 L = positionVS - P;
    half dist = length(L);
    L = L / dist;

    half3 pointLighting = ApplyPointLightImpl(light, L, dist, V, P, N, F0, cdiff, metallic, linearRoughness);
    half spotIntensity = SpotCone(light.innerConeAngle, light.outerConeAngle, directionVS, L);

    return pointLighting * spotIntensity;
}
