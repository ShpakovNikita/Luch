#pragma once

#include <Husky/Flags.h>

namespace Husky::Graphics
{
    enum class BufferUsageFlags
    {
        Unknown = 0,
        TransferSource = 1 << 0,
        TransferDestination = 1 << 1,
        VertexBuffer = 1 << 2,
        IndexBuffer = 1 << 3,
        Uniform = 1 << 4,
        // TODO
    };
}
