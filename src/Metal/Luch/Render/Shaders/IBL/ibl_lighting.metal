#pragma once

#include <metal_stdlib>

using namespace metal;

half3 CalculateIndirectDiffuse(
    texturecube<half> diffuseIlluminanceCube,
    float3 reflectedWS,
    half3 cdiff)
{
    if(is_null_texture(diffuseIlluminanceCube))
    {
        return half3(0.0);
    }

    constexpr sampler diffuseIlluminanceSampler{ filter::linear, min_filter::linear, mag_filter::linear };

    half3 diffuseIlluminance = diffuseIlluminanceCube.sample(diffuseIlluminanceSampler, reflectedWS).rgb;
    return cdiff * M_1_PI_H * diffuseIlluminance;
}

half3 CalculateSpecularReflection(
    texturecube<half> specularReflectionCube,
    texture2d<half> specularBRDF,
    half3 F0,
    float3 reflectedWS,
    half NdotV,
    half metallic,
    half linearRoughness)
{
    if(is_null_texture(specularReflectionCube) || is_null_texture(specularBRDF))
    {
        return half3(0.0);
    }

    // TODO mip_filter::linear doesn't work in compute shaders
    constexpr sampler specularReflectionSampler{ filter::linear, mip_filter::linear };
    constexpr sampler specularBRDFSampler { filter::linear };

    ushort mipLevelCount = specularReflectionCube.get_num_mip_levels();
    half lod = mix(0, half(mipLevelCount), sqrt(linearRoughness));
    half3 prefilteredSpecular = specularReflectionCube.sample(specularReflectionSampler, reflectedWS, level(lod)).rgb;

    half2 brdf = specularBRDF.sample(specularBRDFSampler, float2(linearRoughness, NdotV)).xy;

    return prefilteredSpecular * (F0 * brdf.x + brdf.y);
}
