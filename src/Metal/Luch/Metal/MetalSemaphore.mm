#include <Luch/Metal/MetalSemaphore.h>
#include <Luch/Metal/MetalGraphicsDevice.h>
#include <dispatch/semaphore.h>

namespace Luch::Metal
{
    using namespace Graphics;
    MetalSemaphore::MetalSemaphore(MetalGraphicsDevice* device, int32 value) : Semaphore(device)
    {
        semaphore = dispatch_semaphore_create(value);
    }

    MetalSemaphore::~MetalSemaphore()
    {
        dispatch_release(semaphore);
    }

    bool MetalSemaphore::Wait(int64 timeoutNS)
    {
        return dispatch_semaphore_wait(semaphore, timeoutNS) != 0;
    }

    void MetalSemaphore::Signal()
    {
        dispatch_semaphore_signal(semaphore);
    }
}
