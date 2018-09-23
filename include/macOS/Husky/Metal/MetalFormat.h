#pragma once

#include <Husky/Graphics/Format.h>
#include <Husky/Metal/MetalForwards.h>

namespace Husky::Metal
{
    using namespace Graphics;

    mtlpp::VertexFormat ToMetalVertexFormat(Format format);
    mtlpp::PixelFormat ToMetalPixelFormat(Format format);
}
