#include <Husky/Vulkan/CommandPool.h>
#include <Husky/Vulkan/Device.h>

namespace Husky::Vulkan
{
    Husky::Vulkan::CommandPool::~CommandPool()
    {
        if (device)
        {
            // TODO I don't like this approach. Device created command pool and it should be delegated destroying it.
            device->GetDevice().destroyCommandPool(commandPool, device->GetAllocationCallbacks());
        }
    }

    Husky::Vulkan::CommandPool::CommandPool(CommandPool && other)
        : device(other.device)
        , commandPool(other.commandPool)
    {
        other.device = nullptr;
        other.commandPool = nullptr;
    }

    CommandPool& CommandPool::operator=(CommandPool&& other)
    {
        device = other.device;
        commandPool = other.commandPool;

        other.device = nullptr;
        other.commandPool = nullptr;

        return *this;
    }

    vk::Result CommandPool::Reset(bool releaseResources)
    {
        vk::CommandPoolResetFlags flags;
        if (releaseResources)
        {
            flags |= vk::CommandPoolResetFlagBits::eReleaseResources;
        }

        return device->GetDevice().resetCommandPool(commandPool, flags);
    }

    CommandPool::CommandPool(Device * aDevice, vk::CommandPool aCommandPool)
        : device(aDevice)
        , commandPool(aCommandPool)
    {
    }
}
