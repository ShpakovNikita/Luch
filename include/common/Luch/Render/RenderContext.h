#pragma once

#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/RefPtr.h>

namespace Luch::Render
{
    using namespace Graphics;

    struct RenderContext
    {
        RefPtr<GraphicsDevice> device;
        RefPtr<CommandQueue> commandQueue;
        RefPtr<Swapchain> swapchain;
    };
}
