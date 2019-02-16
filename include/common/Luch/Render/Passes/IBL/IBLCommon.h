#pragma once

#include <Luch/Types.h>
#include <Luch/VectorTypes.h>

namespace Luch::Render::Passes::IBL
{
    struct EnvironmentCubemapUniform
    {
        uint16 face = 0;
        uint16 _padding0 = 0;
        Vec3 _padding1 = Vec3 { 0 };
    };

    static_assert(sizeof(EnvironmentCubemapUniform) % 4 == 0);
}