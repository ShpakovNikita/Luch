#pragma once

#include <Husky/Flags.h>

namespace Husky
{
    enum class ImageAspects
    {
        None = 0,
        Color = 1 << 0,
        Depth = 1 << 1,
        Stencil = 1 << 2,
        DepthStencil = Depth | Stencil
    };

    DEFINE_OPERATORS_FOR_FLAGS_ENUM(ImageAspects);
}
