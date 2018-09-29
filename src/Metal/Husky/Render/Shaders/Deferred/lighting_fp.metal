#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct VertexOut
{
    float4 position [[position]];
    float2 texCoord;
};

fragment float4 fp_main(VertexOut in [[stage_in]])
{
    return float4(1.0, 0.0, 1.0, 1.0);
}
