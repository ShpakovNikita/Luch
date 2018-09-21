#pragma once

#include <Husky/Flags.h>

namespace Husky::Graphics
{
    enum class ColorComponentFlags
    {
        None = 0,
        Red = 1 << 0,
        Green = 1 << 1,
        Blue = 1 << 2,
        Alpha = 1 << 3,
        All = Red | Green | Blue | Alpha
    };

    DEFINE_OPERATORS_FOR_FLAGS_ENUM(ColorComponentFlags);
}
