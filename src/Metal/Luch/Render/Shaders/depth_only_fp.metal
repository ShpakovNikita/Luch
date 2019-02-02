#include <metal_stdlib>
#include <metal_texture>
#include <simd/simd.h>

// This shader is only called and compiled if there's a alpha test

using namespace metal;

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

    half4 baseColor = baseColorMap.sample(baseColorSampler, texCoord);

    if(baseColor.a < material.alphaCutoff)
    {
        discard_fragment();
    }
}

