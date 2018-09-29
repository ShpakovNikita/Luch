#include <Husky/Metal/MetalSemaphore.h>
#include <Husky/Metal/MetalGraphicsDevice.h>

namespace Husky::Metal
{
    using namespace Graphics;
    MetalSemaphore::MetalSemaphore(MetalGraphicsDevice* device) : Semaphore(device)
    {
    }
}
