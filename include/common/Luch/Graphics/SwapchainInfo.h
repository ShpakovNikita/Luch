#pragma once

#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/Format.h>

namespace Luch::Graphics
{
    struct SwapchainInfo
    {
        int32 imageCount = 0;
        Format format = Format::RGBA8Unorm;
        int32 width = 0;
        int32 height = 0;
    };
}
