#pragma once

using namespace metal;

enum class LightType : ushort
{
    LIGHT_POINT = 0,
    LIGHT_SPOT = 1,
    LIGHT_DIRECTIONAL = 2,
};

enum class LightState : ushort
{
    LIGHT_DISABLED = 0,
    LIGHT_ENABLED = 1,
};

struct Light
{
    float4 positionWS;
    float4 directionWS;
    float4 color;
    LightState state;
    LightType type;
    float innerConeAngle;
    float outerConeAngle;
    float range;
    float intensity;
    float padding0;
    float padding1;
    float padding3;
};
