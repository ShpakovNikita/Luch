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

    #if HAS_NORMAL
        float3 normalLS [[ attribute(1) ]];
    #endif

    #if HAS_TANGENT
        #if HAS_BITANGENT_DIRECTION
            float4 tangentLS [[ attribute(2) ]];
        #else
            float3 tangentLS [[ attribute(2) ]];
        #endif
    #endif

    #if HAS_TEXCOORD_0
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

    #if HAS_NORMAL
        float3 normalVS;
    #endif

    #if HAS_TANGENT
        float4 tangentVS;
    #endif

    #if HAS_TEXCOORD_0
        float2 texCoord;
    #endif
};

vertex VertexOut vp_main(
    VertexIn in [[ stage_in ]],
    device CameraUniform& camera [[ buffer(0) ]],
    device MeshUniform& mesh [[ buffer(1) ]])
{
    VertexOut out;

    float4x4 viewModel = camera.view * mesh.model;

    float4 positionVS = viewModel * float4(in.positionLS.xyz, 1.0);

    out.positionVS = positionVS.xyz;

    #if HAS_NORMAL || HAS_TANGENT
        float4x4 normalMatrix = transpose(mesh.inverseModel * camera.inverseView);
    #endif

    #if HAS_NORMAL
        out.normalVS = (normalMatrix * float4(in.normalLS, 0.0)).xyz;
    #endif

    #if HAS_TEXCOORD_0
        out.texCoord = in.texCoord;
    #endif

    #if HAS_TANGENT
        // Normal matrix or viewmodel matrix?
        out.tangentVS.xyz = (normalMatrix * float4(in.tangentLS.xyz, 0.0)).xyz;
        #if HAS_BITANGENT_DIRECTION
            out.tangentVS.w = in.tangentLS.w;
        #else
            out.tangentVS.w = 1.0;
        #endif
    #endif

    out.positionCS = camera.projection * positionVS;

    return out;
}
