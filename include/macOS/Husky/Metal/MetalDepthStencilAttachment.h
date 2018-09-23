#pragma once

#include <Husky/Graphics/StencilOperation.h>
#include <Husky/Metal/MetalForwards.h>
#
namespace Husky::Metal
{
    using namespace Graphics;

    mtlpp::StencilOperation ToMetalStencilOperation(StencilOperation operation);
}
