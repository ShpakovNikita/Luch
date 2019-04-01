#include <Luch/Vulkan/Sync/VulkanSemaphore.h>
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

    bool VulkanSemaphore::Wait(Optional<int64> timeoutNS)
    {
        // todo: implement
        return false;
    }

    void VulkanSemaphore::Signal()
    {
        // todo: implement
    }

}
