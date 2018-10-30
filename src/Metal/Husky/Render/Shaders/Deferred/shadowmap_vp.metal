#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct CameraUniform
{
    float4x4 view;
    float4x4 inverseView;
    float4x4 projection;
    float4x4 inverseProjection;
    float4x4 viewProjection;
    float4x4 inverseViewProjection;
    float4 positionWS;
};

struct MeshUniform
{
    float4x4 model;
    float4x4 inverseModel;
};

struct VertexIn
{
    float3 positionLS [[ attribute(0) ]];
};

struct VertexOut
{
    float4 positionCS [[position]];
};

vertex VertexOut vp_main(
    VertexIn in [[ stage_in ]],
    device CameraUniform& camera [[ buffer(0) ]],
    device MeshUniform& mesh [[ buffer(1) ]])
{
    VertexOut out;

    out.positionCS = camera.projection * camera.view * mesh.model * float4(in.positionLS.xyz, 1.0);

    return out;
}
