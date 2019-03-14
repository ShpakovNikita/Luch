#include <metal_stdlib>
#include <simd/simd.h>

#include "Common/cubemap.metal"
#include "Common/lighting.metal"
#include "IBL/ibl.metal"

using namespace metal;

struct SpecularReflectionParams
{
    ushort mipLevel;
    ushort mipLevelCount;
};

half3 PrefilteredSpecularReflection(
    texturecube<half> luminanceMap,
    half linearRoughness,
    half3 N,
    ushort sampleCount)
{
    constexpr sampler luminanceSampler{ filter::linear };

    half3 V = N;

    half3 luminance = 0;
    half totalWeight = 0;

    half a = linearRoughness * linearRoughness;
    half a2 = a  * a;

    for(ushort i = 0; i < sampleCount; i++)
    {

        half2 Xi = half2(Hammersley(i, sampleCount));
        half3 H = ImportanceSampleGGX(Xi, a2, N);
        half3 L = reflect(-V, H);

        half NdotL = saturate(dot(N, L));
        if(NdotL > 0)
        {
            luminance += luminanceMap.sample(luminanceSampler, float3(N)).rgb * NdotL;
            totalWeight += NdotL;
        }
    }

    luminance /= totalWeight;

    return luminance;
}

kernel void specular_reflection_kernel(
    ushort3 gid [[thread_position_in_grid]],
    texturecube<half> luminanceMap [[ texture(0) ]],
    texturecube<half, access::write> specularReflectionMap [[ texture(1) ]],
    constant SpecularReflectionParams& params [[ buffer(0) ]])
{
    constexpr sampler s;
    ushort face = gid.z;

    constexpr ushort sampleCount = 64;

    ushort mipLevel = params.mipLevel;
    ushort mipLevelCount = params.mipLevelCount;
    half linearRoughnessSqrt = mipLevel / (mipLevelCount - 1);
    half linearRoughness = linearRoughnessSqrt * linearRoughnessSqrt;
    half2 invOutputSize = half2(1.0h / specularReflectionMap.get_width(), 1.0h / specularReflectionMap.get_height());

    half3 N = CubemapDirection(gid.xy, face, invOutputSize);

    half3 luminance = PrefilteredSpecularReflection(luminanceMap, linearRoughness, N, sampleCount);

    specularReflectionMap.write(half4(luminance, 1.0), gid.xy, face);
}
