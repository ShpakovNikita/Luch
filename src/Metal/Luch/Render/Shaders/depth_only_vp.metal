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

    #if ALPHA_MASK
        float2 texCoord [[ attribute(3) ]];
    #endif
};

// WS - world space
// VS - view space
// LS - local space
// CS - clip space

struct VertexOut
{
    float4 positionCS [[ position ]];
    float3 positionVS;

    #if ALPHA_MASK
        float2 texCoord;
    #endif
};

vertex VertexOut vp_main(
    VertexIn in [[ stage_in ]],
    constant CameraUniform& camera [[ buffer(0) ]],
    constant MeshUniform& mesh [[ buffer(1) ]])
{
    VertexOut out;

    float4x4 viewModel = camera.view * mesh.model;

    float4 positionVS = viewModel * float4(in.positionLS.xyz, 1.0);

    out.positionVS = positionVS.xyz;

    #if ALPHA_MASK
        out.texCoord = in.texCoord;
    #endif

    out.positionCS = camera.projection * positionVS;

    return out;
}
