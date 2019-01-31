#pragma once

#include <Luch/Flags.h>

namespace Luch::Graphics
{
    enum class ShaderStage
    {
        Fragment = 1 << 1,
        Vertex = 1 << 2,
        Compute = 1 << 3,
        Tile = 1 << 4,
    };

    DEFINE_OPERATORS_FOR_FLAGS_ENUM(ShaderStage);
}
