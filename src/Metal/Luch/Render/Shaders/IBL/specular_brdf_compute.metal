#include <metal_stdlib>
#include <simd/simd.h>

#include "Common/cubemap.metal"
#include "Common/lighting.metal"
#include "IBL/ibl.metal"

using namespace metal;

half2 ApproximateBRDF(half roughness, half NdotV)
{
    // Karis' approximation based on Lazarov's
    constexpr half4 c0 = half4(-1.0, -0.0275, -0.572,  0.022);
    constexpr half4 c1 = half4( 1.0,  0.0425,  1.040, -0.040);
    half4 r = roughness * c0 + c1;
    half a004 = min(r.x * r.x, exp2(-9.28h * NdotV)) * r.x + r.y;
    return half2(-1.04, 1.04) * a004 + r.zw;
    // Zioma's approximation based on Karis
    // return vec2(1.0, pow(1.0 - max(roughness, NoV), 3.0));
}

half3 IntegrateBRDF(half linearRoughness, half NdotV, ushort sampleCount)
{
    constexpr half minRoughness = 0.001;

    half3 V = half3(sqrt(1 - NdotV * NdotV), 0, NdotV);
    half3 N = half3(0, 0, 1);

    half3 result = 0;

    half a = max(linearRoughness * linearRoughness, minRoughness);
    half a2 = a  * a;

    half clampedNdotV = max(NdotV, 0.000001h);

    for(ushort i = 0; i < sampleCount; i++)
    {
        half2 Xi = half2(Hammersley(i, sampleCount));

        // specular term
        {
            half3 H = ImportanceSampleGGX(Xi, a2, N);
            half3 L = reflect(-V, H);

            half NdotL = saturate(dot(N, L));
            half NdotH = saturate(dot(N, H));
            half VdotH = saturate(dot(V, H));

            if(NdotL > 0)
            {
                half G = G_SmithGGXCorrelated(NdotL, clampedNdotV, a2);
                half Gv = G * VdotH / (clampedNdotV * NdotH);
                half Fc = pow(1 - VdotH, 5);
                result.x += Gv * (1 - Fc);
                result.y += Gv * Fc;
            }
        }

        // Big todo if I wanna use some complex diffuse term
        // // Disney diffuse term
        // {
        //     half3 L = ImportanceSampleCos(Xi, N);

        //     half NdotL = saturate(dot(N, L));

        //     if(NdotL > 0)
        //     {
        //         // TODO optimize, we only need LdotH
        //         half3 H = normalize(V + L);
        //         half NdotV = saturate(dot(N, V));
        //         half LdotH = saturate(dot(L, H));
        //         result.z += Fr_DisneyDiffuse(NdotV, NdotL, LdotH, linearRoughness) * M_PI_H;
        //     }
        // }
    }

    return result / sampleCount;
}

kernel void brdf_kernel(
    ushort2 gid [[thread_position_in_grid]],
    texture2d<half, access::write> brdfTexture [[ texture(0) ]])
{
    constexpr ushort sampleCount = 128;
    half NdotV = half(gid.x) / brdfTexture.get_width();
    half linearRoughness = half(gid.y) / brdfTexture.get_height();

    half3 dfg = IntegrateBRDF(linearRoughness, NdotV, sampleCount);

    brdfTexture.write(half4(dfg, 0), gid);
}
