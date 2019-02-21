#include <metal_stdlib>
#include <simd/simd.h>

#include "Common/cubemap.metal"
#include "Common/lighting.metal"
#include "IBL/ibl.metal"

using namespace metal;

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

        if(NdotL > 0)
        {
            // TODO check and understand math here
            half G = G_CookTorranceGGX(VdotH, NdotH, NdotV, NdotL);

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