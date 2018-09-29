#pragma once

#include <Husky/Graphics/GraphicsResult.h>
#include <Husky/Metal/MetalForwards.h>

namespace Husky::Metal
{
    using namespace Graphics;

    GraphicsResult LibraryErrorToGraphicsResult(ns::Error error);
    GraphicsResult PipelineErrorToGraphicsResult(ns::Error error);
}
