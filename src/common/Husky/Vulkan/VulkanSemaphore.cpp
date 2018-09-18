#include <Husky/Vulkan/Semaphore.h>
#include <Husky/Vulkan/GraphicsDevice.h>

namespace Husky::Vulkan
{
    VulkanSemaphore::VulkanSemaphore(VulkanGraphicsDevice* aDevice, vk::Semaphore aSemaphore)
        : device(aDevice)
        , semaphore(aSemaphore)
    {
    }

    VulkanSemaphore:~VulkanSemaphore()
    {
        Destroy();
    }

    void VulkanSemaphore::Destroy()
    {
        if (device)
        {
            device->DestroySemaphore(this);
        }
    }

}
