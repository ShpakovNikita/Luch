#pragma once

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

constant constexpr array<half3, 6> CubemapNormal = 
{
    half3{ 1, 0, 0 },
    half3{ -1, 0, 0 },
    half3{ 0, 1, 0 },
    half3{ 0, -1, 0 },
    half3{ 0, 0, 1 },
    half3{ 0, 0, -1 },
};

constant constexpr array<half3, 6> CubemapU =
{
    half3{ 0, 0, -1 },
    half3{ 0, 0, 1 },
    half3{ 1, 0, 0 },
    half3{ 1, 0, 0 },
    half3{ 1, 0, 0 },
    half3{ -1, 0, 0 },
};

constant constexpr array<half3, 6> CubemapV = 
{
    half3{ 0, 1, 0 },
    half3{ 0, 1, 0 },
    half3{ 0, 0, -1 },
    half3{ 0, 0, 1 },
    half3{ 0, 1, 0 },
    half3{ 0, 1, 0 },
};

half3 CubemapDirection(ushort2 coords, ushort face, half2 invSize)
{
    return normalize(
        CubemapNormal[face]
        + mix(-1, 1, coords.x * invSize.x) * CubemapU[face]
        + mix(1, -1, coords.y * invSize.y) * CubemapV[face]);
}
