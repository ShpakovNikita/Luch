#pragma once

#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/Graphics/Format.h>

namespace Husky::Graphics
{
    struct SwapchainInfo
    {
        int32 imageCount = 0;
        Format format = Format::R8G8B8A8Unorm;
        int32 width = 0;
        int32 height = 0;
    };
}
