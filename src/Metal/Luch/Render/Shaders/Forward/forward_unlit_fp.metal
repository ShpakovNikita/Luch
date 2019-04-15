#include <metal_stdlib>
#include <metal_texture>
#include <simd/simd.h>

#include "Common/camera.metal"
#include "Common/material.metal"

using namespace metal;

struct VertexOut
{
    float4 positionCS [[position]];
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

struct FragmentOut
{
    half4 luminance [[color(0)]];
};

// Figure out coordinate system
#if !ALPHA_MASK
[[early_fragment_tests]]
#endif
fragment FragmentOut FragmentMain(
    VertexOut in [[stage_in]],
    constant CameraUniform& camera [[ buffer(0) ]],
    constant MaterialUniform& material [[ buffer(1) ]]

#if HAS_BASE_COLOR_TEXTURE
    , texture2d<half> baseColorMap [[ texture(0) ]]         // RGB - color, A - opacity
    , sampler baseColorSampler [[ sampler(0) ]]
#endif
    )
{
    half4 baseColor = half4(material.baseColorFactor);

    #if HAS_BASE_COLOR_TEXTURE && HAS_TEXCOORD_0
        float2 texCoord = in.texCoord;
        half4 baseColorSample = baseColorMap.sample(baseColorSampler, texCoord);
        baseColor *= baseColorSample;
    #endif

    #if ALPHA_MASK
        if(baseColor.a < material.alphaCutoff)
        {
            discard_fragment();
        }
    #endif

    FragmentOut result;
    result.luminance = baseColor;
    return result;
}
