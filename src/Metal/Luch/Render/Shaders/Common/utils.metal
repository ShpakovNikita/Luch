#pragma once

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

float3 ExtractNormal(float3 normalTS, float normalScale, float3x3 TBN)
{
    float3 result = (normalTS * 2 - float3(1.0)) * float3(normalScale, normalScale, 1.0);

    return normalize(TBN * result);
}

float3x3 TangentFrame(float3 dp1, float3 dp2, float3 N, float2 uv)
{
    // get edge vectors of the pixel triangle
    float2 duv1 = dfdx(uv);
    float2 duv2 = dfdy(uv);

    // solve the linear system
    float3 dp2perp = cross(dp2, N);
    float3 dp1perp = cross(N, dp1);
    float3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    float3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    // construct a scale-invariant frame
    float invmax = rsqrt(max(dot(T, T), dot(B,B)));
    return float3x3(T * invmax, B * invmax, N);
}
