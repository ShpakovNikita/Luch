#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct VertexIn
{
    float3 positionLS [[attribute(0)]];
    float2 texCoord [[attribute(1)]];
};

struct VertexOut
{
    float4 positionCS [[position]];
    float2 texCoord;
};

vertex VertexOut vp_main(VertexIn in [[stage_in]])
{
    VertexOut out;
    out.texCoord = in.texCoord;
    out.positionCS = float4(in.positionLS, 1.0);
    return out;
}
