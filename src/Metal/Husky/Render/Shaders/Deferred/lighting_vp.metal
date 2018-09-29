
#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Vertex
{
    float3 position [[attribute(0)]];
    float2 texCoord [[attribute(1)]];
};

struct VertexOut
{
    float4 position [[position]];
    float2 texCoord;
};

vertex VertexOut vp_main(Vertex in [[stage_in]])
{
    VertexOut out;

    return out;
}
