#pragma once

#include <Husky/Flags.h>

namespace Husky::Graphics
{
    enum class TextureUsageFlags
    {
        Unknown = 0,
        ShaderRead = 1 << 0,
        ShaderWrite = 1 << 1,
        RenderTarget = 1 << 2,
    };

    DEFINE_OPERATORS_FOR_FLAGS_ENUM(TextureUsageFlags);
}
