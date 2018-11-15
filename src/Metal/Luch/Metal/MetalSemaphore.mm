#include <Luch/Metal/MetalSemaphore.h>
#include <Luch/Metal/MetalGraphicsDevice.h>

namespace Luch::Metal
{
    using namespace Graphics;
    MetalSemaphore::MetalSemaphore(MetalGraphicsDevice* device) : Semaphore(device)
    {
    }
}
