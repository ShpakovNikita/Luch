#pragma once

#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/RefPtr.h>

namespace Luch::Render
{
    using namespace Graphics;

    struct RenderContext
    {
        RefPtr<PhysicalDevice> physicalDevice;
        RefPtr<GraphicsDevice> device;
        RefPtr<CommandQueue> commandQueue;
        RefPtr<Surface> surface;
        RefPtr<Swapchain> swapchain;
    };
}
