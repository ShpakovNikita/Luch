#pragma once

#include <Luch/Types.h>
#include <Luch/VectorTypes.h>

namespace Luch::Render::CubemapCommon
{
    constexpr Array<Vec3, 6> CubemapNormal = 
    {
        Vec3{ 1, 0, 0 },
        Vec3{ -1, 0, 0 },
        Vec3{ 0, 1, 0 },
        Vec3{ 0, -1, 0 },
        Vec3{ 0, 0, 1 },
        Vec3{ 0, 0, -1 },
    };

    constexpr Array<Vec3, 6> CubemapUp = 
    {
        Vec3{ 0, 1, 0 },
        Vec3{ 0, 1, 0 },
        Vec3{ 0, 0, -1 },
        Vec3{ 0, 0, 1 },
        Vec3{ 0, 1, 0 },
        Vec3{ 0, 1, 0 },
    };

    constexpr Array<Vec3, 6> CubemapRight = 
    {
        Vec3{ 0, 0, -1 },
        Vec3{ 0, 0, 1 },
        Vec3{ 1, 0, 0 },
        Vec3{ 1, 0, 0 },
        Vec3{ 1, 1, 0 },
        Vec3{ -1, 0, 0 },
    };
}
