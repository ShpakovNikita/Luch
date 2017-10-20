#pragma once

#include <Husky/Flags.h>

namespace Husky
{
    enum class ShaderStage
    {
        Fragment = 1 << 0,
        Geometry = 1 << 1,
        TessellationControl = 1 << 2,    // hull
        TessellationEvaluation = 1 << 3, // domain
        Vertex = 1 << 4,
        Compute = 1 << 5,
    };

    DEFINE_OPERATORS_FOR_FLAGS_ENUM(ShaderStage)
}
