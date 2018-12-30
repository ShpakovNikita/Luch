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
    half4 color [[color(0)]];
};

fragment FragmentOut fp_main(
    VertexOut in [[stage_in]],
    texture2d<float> luminanceMap [[texture(0)]])
{
    constexpr sampler luminanceSampler(coord::normalized);
    FragmentOut result;

    float2 texCoord = in.texCoord;
    float4 colorSample = luminanceMap.sample(luminanceSampler, texCoord);

    result.color = half4(colorSample);

    return result;
}
