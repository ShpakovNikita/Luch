#pragma once

#include <metal_stdlib>

using namespace metal;

half3 CalculateIndirectDiffuse(
    texturecube<half> diffuseIrradianceMap,
    float3 reflectedWS,
    half3 baseColor,
    half metallic)
{
    if(is_null_texture(diffuseIrradianceMap))
    {
        return half3(0.0);
    }

    constexpr sampler diffuseIrradianceSampler{ filter::linear, min_filter::linear, mag_filter::linear };

    half3 diffuseIrradiance = diffuseIrradianceMap.sample(diffuseIrradianceSampler, reflectedWS).rgb;
    return (1 - metallic) * baseColor * M_1_PI_H * diffuseIrradiance;
}

half3 CalculateSpecularReflection(
    texturecube<half> specularReflectionMap,
    texture2d<half> specularBRDF,
    half3 F0,
    float3 reflectedWS,
    half NdotV,
    half metallic,
    half roughness)
{
    if(is_null_texture(specularReflectionMap) || is_null_texture(specularBRDF))
    {
        return half3(0.0);
    }

    // TODO mip_filter::linear doesn't work in compute shaders
    constexpr sampler specularReflectionSampler{ filter::linear, mip_filter::linear };
    constexpr sampler specularBRDFSampler { filter::linear };

    ushort mipLevelCount = specularReflectionMap.get_num_mip_levels();
    half lod = mix(0, half(mipLevelCount), roughness);
    half3 prefilteredSpecular = specularReflectionMap.sample(specularReflectionSampler, reflectedWS, level(lod)).rgb;

    half2 brdf = specularBRDF.sample(specularBRDFSampler, float2(roughness, NdotV)).xy;

    return prefilteredSpecular * (F0 * brdf.x + brdf.y);
}
