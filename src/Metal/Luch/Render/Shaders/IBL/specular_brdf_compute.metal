#include <metal_stdlib>
#include <simd/simd.h>

#include "Common/cubemap.metal"
#include "Common/lighting.metal"
#include "IBL/ibl.metal"

using namespace metal;

half GDFG(half NdotV, half NdotL, half roughness)
{
    float a2 = roughness * roughness;
    float GGXL = NdotV * sqrt((-NdotL * a2 + NdotL) * NdotL + a2);
    float GGXV = NdotL * sqrt((-NdotV * a2 + NdotV) * NdotV + a2);
    return (2 * NdotL) / (GGXV + GGXL);
}

half2 IntegrateBRDF(half roughness, half NdotV, ushort sampleCount)
{
    half3 V = half3(sqrt(1 - NdotV * NdotV), 0, NdotV);
    half3 N = half3(0, 0, 1);

    half2 result;

    for(ushort i = 0; i < sampleCount; i++)
    {
        half2 Xi = half2(Hammersley(i, sampleCount));
        half3 H = ImportanceSampleGGX(Xi, roughness, N);
        half3 L = reflect(-V, H);

        half NdotL = saturate(dot(N, L));
        half NdotH = saturate(dot(N, H));
        half VdotH = saturate(dot(V, H));
        half NdotV = saturate(dot(N, V));

        if(NdotL > 0)
        {
            // TODO check and understand math here
            half G = GDFG(NdotV, NdotL, roughness);

            half Gv = G * VdotH / NdotH;
            half Fc = pow(1 - VdotH, 5);
            result.x += Gv * (1 - Fc);
            result.y += Gv * Fc;
        }
    }

    return result / sampleCount;
}

kernel void brdf_kernel(
    ushort2 gid [[thread_position_in_grid]],
    texture2d<half, access::write> brdfTexture [[ texture(0) ]])
{
    constexpr ushort sampleCount = 128;
    half NdotV = half(gid.x) / brdfTexture.get_width();
    half roughness = half(gid.y) / brdfTexture.get_height();

    half2 dfg = IntegrateBRDF(roughness, NdotV, sampleCount);

    brdfTexture.write(half4(dfg, 0, 0), gid);
}
