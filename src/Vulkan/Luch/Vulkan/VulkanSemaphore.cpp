#include <Luch/Vulkan/VulkanSemaphore.h>
#include <Luch/Vulkan/VulkanGraphicsDevice.h>

namespace Luch::Vulkan
{
    VulkanSemaphore::VulkanSemaphore(
        VulkanGraphicsDevice* aDevice,
        vk::Semaphore aSemaphore)
        : Semaphore(aDevice)
        , device(aDevice)
        , semaphore(aSemaphore)
    {
    }

    VulkanSemaphore::~VulkanSemaphore()
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
