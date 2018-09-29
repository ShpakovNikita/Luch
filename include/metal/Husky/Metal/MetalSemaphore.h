#pragma once

#include <Husky/Graphics/Semaphore.h>
#include <Husky/Metal/MetalForwards.h>
#include <dispatch/dispatch.h>

namespace Husky::Metal
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
