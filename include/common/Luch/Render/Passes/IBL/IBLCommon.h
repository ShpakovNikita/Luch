#pragma once

#include <Luch/Types.h>
#include <Luch/VectorTypes.h>

namespace Luch::Render::Passes::IBL
{
    struct SpecularReflectionParamsUniform
    {
        uint16 mipLevel = 0;
        uint16 mipLevelCount = 0;
        Vec3 _padding1 = Vec3 { 0 };
    };

    static_assert(sizeof(SpecularReflectionParamsUniform) % 4 == 0);

    struct EnvironmentCubemapUniform
    {
        uint16 face = 0;
        uint16 _padding0 = 0;
        Vec3 _padding1 = Vec3 { 0 };
    };

    static_assert(sizeof(EnvironmentCubemapUniform) % 4 == 0);
}