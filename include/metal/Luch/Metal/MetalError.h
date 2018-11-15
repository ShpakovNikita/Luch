#pragma once

#include <Luch/Graphics/GraphicsResult.h>
#include <Luch/Metal/MetalForwards.h>

namespace Luch::Metal
{
    using namespace Graphics;

    GraphicsResult LibraryErrorToGraphicsResult(ns::Error error);
    GraphicsResult PipelineErrorToGraphicsResult(ns::Error error);
}
