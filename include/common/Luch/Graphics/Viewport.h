#pragma once

#include <Luch/Types.h>

namespace Luch::Graphics
{
    struct Viewport
    {
        float32 x = 0.0f;
        float32 y = 0.0f;
        float32 width = 0.0f;
        float32 height = 0.0f;
        float32 minDepth = 0.0f;
        float32 maxDepth = 1.0f;
    };
}
