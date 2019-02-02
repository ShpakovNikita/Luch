#include <Luch/Vulkan/VulkanFence.h>
#include <Luch/Vulkan/VulkanGraphicsDevice.h>

namespace Luch::Vulkan
{
    VulkanFence::VulkanFence(
        VulkanGraphicsDevice* aDevice,
        vk::Fence aFence)
        : device(aDevice)
        , fence(aFence)
    {
    }

    VulkanFence::~VulkanFence()
    {
        Destroy();
    }

    GraphicsResultValue<FenceWaitResult> VulkanFence::Wait(Optional<Timeout> timeout)
    {
        auto waitResult = device->device.waitForFences({ fence }, VK_TRUE, ToVulkanTimeout(timeout));
        switch (waitResult)
        {
        case vk::Result::eSuccess:
            return { vk::Result::eSuccess, FenceWaitResult::Signaled };
        case vk::Result::eTimeout:
            return { vk::Result::eSuccess, FenceWaitResult::Timeout }; // hm
        default:
            return { waitResult };
        }
    }

    GraphicsResultValue<FenceStatus> VulkanFence::GetStatus()
    {
        auto result = device->device.getFenceStatus(fence);
        switch (result)
        {
        case vk::Result::eSuccess:
            return { vk::Result::eSuccess, FenceStatus::Signaled };
        case vk::Result::eNotReady:
            return { vk::Result::eSuccess, FenceStatus::Unsignaled }; // hm
        default:
            return { result };
        }
    }

    vk::Result VulkanFence::Reset()
    {
        return device->device.resetFences({ fence });
    }

    void VulkanFence::Destroy()
    {
        if (device)
        {
            device->DestroyFence(this);
        }
    }

}
