#pragma once

#include <Luch/Flags.h>

namespace Luch::Graphics
{
    enum class TextureUsageFlags
    {
        Unknown = 0,
        ShaderRead = 1 << 0,
        ShaderWrite = 1 << 1,
        ColorAttachment = 1 << 2,
        DepthStencilAttachment = 1 << 3,
        TransferSource = 1 << 4,
        TransferDestination = 1 << 5,
    };

    DEFINE_OPERATORS_FOR_FLAGS_ENUM(TextureUsageFlags);
}
