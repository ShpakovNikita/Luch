#include <Husky/Vulkan/Semaphore.h>
#include <Husky/Vulkan/GraphicsDevice.h>

namespace Husky::Vulkan
{
    Semaphore::Semaphore(GraphicsDevice* aDevice, vk::Semaphore aSemaphore)
        : device(aDevice)
        , semaphore(aSemaphore)
    {
    }

    Semaphore::Semaphore(Semaphore&& other)
        : device(other.device)
        , semaphore(other.semaphore)
    {
        other.device = nullptr;
        other.semaphore = nullptr;
    }

    Semaphore& Semaphore::operator=(Semaphore&& other)
    {
        Destroy();

        device = other.device;
        semaphore = other.semaphore;

        other.device = nullptr;
        other.semaphore = nullptr;

        return *this;
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
