#pragma once

#include <metal_stdlib>

using namespace metal;

struct MaterialUniform
{
    packed_float4 baseColorFactor;
    packed_float3 emissiveFactor;
    uint32_t unlit = 0;
    float alphaCutoff;
    float metallicFactor;
    float roughnessFactor;
    float normalScale;
    float occlusionStrength;
};
