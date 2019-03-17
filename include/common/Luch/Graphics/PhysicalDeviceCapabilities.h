#pragma once

#include <Luch/Types.h>
#include <Luch/Graphics/Format.h>
#include <Luch/Graphics/GraphicsForwards.h>

namespace Luch::Graphics
{
    struct PhysicalDeviceCapabilities
    {
        bool hasTileBasedArchitecture = false;
        Vector<Format> supportedDepthFormats;
    };
}
