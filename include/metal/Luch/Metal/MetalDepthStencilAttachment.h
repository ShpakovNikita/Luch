#pragma once

#include <Luch/Graphics/StencilOperation.h>
#include <Luch/Graphics/CompareFunction.h>
#include <Luch/Metal/MetalForwards.h>

namespace Luch::Metal
{
    using namespace Graphics;

    mtlpp::StencilOperation ToMetalStencilOperation(StencilOperation operation);
    mtlpp::CompareFunction ToMetalCompareFunction(CompareFunction compareFunction);
}
