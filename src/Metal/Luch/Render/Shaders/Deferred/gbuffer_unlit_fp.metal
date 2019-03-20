#include <metal_stdlib>
#include <metal_texture>
#include <simd/simd.h>

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
    half4 gbuffer0 [[ color(0) ]];
    half4 gbuffer1 [[ color(1) ]];
    half4 gbuffer2 [[ color(2) ]];
};

// Figure out coordinate system
#if !ALPHA_MASK
[[early_fragment_tests]]
#endif
fragment FragmentOut fp_main(
    VertexOut in [[stage_in]],
    constant MaterialUniform& material [[ buffer(0) ]]

#if HAS_BASE_COLOR_TEXTURE
    , texture2d<half> baseColorMap [[ texture(0) ]]         // RGB - color, A - opacity
    , sampler baseColorSampler [[ sampler(0) ]]
#endif
    )
{
    FragmentOut out;

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

    out.gbuffer0.rgb = baseColor.rgb;
    out.gbuffer0.a = 0;

    out.gbuffer1.rgba = 0;
    out.gbuffer2.rgba = 0;

    return out;
}

