#pragma once

#include <Luch/Graphics/Semaphore.h>
#include <Luch/Metal/MetalForwards.h>
#include <dispatch/dispatch.h>

namespace Luch::Metal
{
    using namespace Graphics;

    class MetalSemaphore : public Semaphore
    {
        friend class MetalSwapchain;
        friend class MetalCommandQueue;
    public:
        MetalSemaphore(MetalGraphicsDevice* device);
    private:
        dispatch_semaphore_t semaphore;
    };
}
