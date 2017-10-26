#include <Husky/Vulkan/Fence.h>
#include <Husky/Vulkan/GraphicsDevice.h>

namespace Husky::Vulkan
{
    Fence::Fence(GraphicsDevice* aDevice, vk::Fence aFence)
        : device(aDevice)
        , fence(aFence)
    {
    }

    Fence::Fence(Fence&& other)
        : device(other.device)
        , fence(other.fence)
    {
        other.device = nullptr;
        other.fence = nullptr;
    }

    Fence& Fence::operator=(Fence&& other)
    {
        Destroy();

        device = other.device;
        fence = other.fence;

        other.device = nullptr;
        other.fence = nullptr;

        return *this;
    }

    VulkanResultValue<FenceWaitResult> Fence::Wait(Optional<Timeout> timeout)
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

    VulkanResultValue<FenceStatus> Fence::GetStatus()
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

    vk::Result Fence::Reset()
    {
        return device->device.resetFences({ fence });
    }

    Fence::~Fence()
    {
        Destroy();
    }

    void Fence::Destroy()
    {
        if (device)
        {
            device->DestroyFence(this);
        }
    }

}
