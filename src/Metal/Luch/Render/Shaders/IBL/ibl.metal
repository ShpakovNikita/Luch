#pragma once

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

float2 Hammersley(uint i, uint sampleCount)
{
    uint bits = i;
    bits = (bits << 16) | (bits >> 16);
    bits = ((bits & 0x55555555) << 1) | ((bits & 0xAAAAAAAA) >> 1);
    bits = ((bits & 0x33333333) << 2) | ((bits & 0xCCCCCCCC) >> 2);
    bits = ((bits & 0x0F0F0F0F) << 4) | ((bits & 0xF0F0F0F0) >> 4);
    bits = ((bits & 0x00FF00FF) << 8) | ((bits & 0xFF00FF00) >> 8);
    return float2(float(i) / sampleCount, bits / exp2(32.0));
}

half3 ImportanceSampleGGXTangent(half2 Xi, half a2)
{
    half phi = 2 * M_PI_H * Xi.x;
    half cosTheta = sqrt((1 - Xi.y) / (1 + (a2 - 1) * Xi.y));
    half sinTheta = sqrt(1 - cosTheta * cosTheta);

    return half3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
}

half3 ImportanceSampleGGX(half2 Xi, half a2, half3 N)
{
    half3 H = ImportanceSampleGGXTangent(Xi, a2);
    half3 up = abs(N.z) < 0.999 ? half3(0, 0, 1) : half3(1, 0, 0);
    // Workaround, for some reason normalize(half3(1.0, 0.0, 0.0)) returns (inf, 0, 0)
    half3 tangentX = half3(normalize(float3(cross(up, N))));
    half3 tangentY = cross(N, tangentX);
    return tangentX * H.x + tangentY * H.y + N * H.z;
}

half3 ImportanceSampleCosTangent(half2 Xi)
{
    half r = sqrt(Xi.x);
    half phi = Xi.y * M_PI_H * 2;
    return half3(r * cos(phi), r * sin(phi), sqrt(1 - Xi.x));
}

half3 ImportanceSampleCos(half2 Xi, half3 N)
{
    half3 L = ImportanceSampleCosTangent(Xi);
    half3 up = abs(N.z) < 0.999 ? half3(0, 0, 1) : half3(1, 0, 0);
    // Workaround, for some reason normalize(half3(1.0, 0.0, 0.0)) returns (inf, 0, 0)
    half3 tangentX = half3(normalize(float3(cross(up, N))));
    half3 tangentY = cross(N, tangentX);
    return tangentX * L.x + tangentY * L.y + N * L.z;
}
