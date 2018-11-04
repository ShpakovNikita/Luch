#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct VertexOut
{
    float4 positionCS [[position]];
    float2 texCoord;
};

struct FragmentOut
{
    float4 color [[color(0)]];
};

fragment FragmentOut fp_main(
    VertexOut in [[stage_in]],
    texture2d<float> baseColorMap [[texture(0)]],
    texture2d<float> diffuseLighting [[texture(1)]],
    texture2d<float> specularLighting [[texture(2)]],
    sampler baseColorSampler [[sampler(0)]],
    sampler diffuseSampler [[sampler(1)]],
    sampler specularSampler [[sampler(2)]])
{
    FragmentOut result;
    float2 texCoord = in.texCoord;

    float4 baseColorSample = baseColorMap.sample(baseColorSampler, texCoord);
    float4 diffuseLightingSample = diffuseLighting.sample(diffuseSampler, texCoord);
    float4 specularLightingSample = specularLighting.sample(specularSampler, texCoord);

    result.color.xyz = baseColorSample.xyz * (diffuseLightingSample.xyz + specularLightingSample.xyz);
    result.color.w = 1.0;

    return result;
}
