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
    texture2d<float> colorMap [[texture(0)]],
    sampler colorSampler [[sampler(0)]])
{
    FragmentOut result;

    float2 texCoord = in.texCoord;
    float4 colorSample = colorMap.sample(colorSampler, texCoord);

    result.color = colorSample;

    return result;
}
