#pragma once

#include <Luch/Graphics/Format.h>
#include <Luch/Metal/MetalForwards.h>

namespace Luch::Metal
{
    using namespace Graphics;

    mtlpp::VertexFormat ToMetalVertexFormat(Format format);
    mtlpp::PixelFormat ToMetalPixelFormat(Format format);
}
