#include <Husky/Vulkan/CommandPool.h>
#include <Husky/Vulkan/GraphicsDevice.h>

namespace Husky::Vulkan
{
    CommandPool::~CommandPool()
    {
        if (device)
        {
            device->DestroyCommandPool(this);
        }
    }

    CommandPool::CommandPool(CommandPool && other)
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

    CommandPool::CommandPool(GraphicsDevice * aDevice, vk::CommandPool aCommandPool)
        : device(aDevice)
        , commandPool(aCommandPool)
    {
    }
}
