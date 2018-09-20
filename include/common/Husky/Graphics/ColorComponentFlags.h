#pragma once

#include <Husky/Flags.h>

namespace Husky::Graphics
{
    enum class ColorComponentFlags
    {
        None = 0,
        Red = 1 << 1,
        Green = 1 << 2,
        Blue = 1 << 3,
        Alpha = 1 << 4,
        All = Red | Green | Blue | Alpha
    };

    DEFINE_OPERATORS_FOR_FLAGS_ENUM(ColorComponentFlags);
}
