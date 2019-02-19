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
        MetalSemaphore(MetalGraphicsDevice* device, int32 value);
        ~MetalSemaphore();
        bool Wait(Optional<int64> timeoutNS = {}) override;
        void Signal() override;
    private:
        dispatch_semaphore_t semaphore;
    };
}
