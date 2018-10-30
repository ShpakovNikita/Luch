#pragma once

#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/RefPtr.h>

namespace Husky::Render
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
