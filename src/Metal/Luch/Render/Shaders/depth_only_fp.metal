#include <metal_stdlib>
#include <metal_texture>
#include <simd/simd.h>

#include "Common/material.metal"

// This shader is only called and compiled if there's a alpha test

using namespace metal;

struct VertexOut
{
    float4 positionCS [[position]];
    float2 texCoord;
};

// Figure out coordinate system
fragment void fp_main(
    VertexOut in [[stage_in]],
    constant MaterialUniform& material [[buffer(0)]],
    texture2d<half> baseColorMap [[texture(0)]],         // RGB - color, A - opacity
    sampler baseColorSampler [[sampler(0)]]
    )
{
    float2 texCoord = in.texCoord;

    half4 baseColorFactor = half4(material.baseColorFactor);
    half4 baseColor = baseColorMap.sample(baseColorSampler, texCoord) * baseColorFactor;

    if(baseColor.a < material.alphaCutoff)
    {
        discard_fragment();
    }
}

