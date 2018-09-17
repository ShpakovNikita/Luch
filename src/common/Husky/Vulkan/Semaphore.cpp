#include <Husky/Vulkan/Semaphore.h>
#include <Husky/Vulkan/GraphicsDevice.h>

namespace Husky::Vulkan
{
    Semaphore::Semaphore(GraphicsDevice* aDevice, vk::Semaphore aSemaphore)
        : device(aDevice)
        , semaphore(aSemaphore)
    {
    }

    Semaphore::~Semaphore()
    {
        Destroy();
    }

    void Semaphore::Destroy()
    {
        if (device)
        {
            device->DestroySemaphore(this);
        }
    }

}
