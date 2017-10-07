#include <Husky/Vulkan/CommandPool.h>
#include <Husky/Vulkan/GraphicsDevice.h>

namespace Husky::Vulkan
{
    CommandPool::CommandPool(GraphicsDevice * aDevice, vk::CommandPool aCommandPool)
        : device(aDevice)
        , commandPool(aCommandPool)
    {
    }

    CommandPool::CommandPool(CommandPool&& other)
        : device(other.device)
        , commandPool(other.commandPool)
    {
        other.device = nullptr;
        other.commandPool = nullptr;
    }

    CommandPool& CommandPool::operator=(CommandPool&& other)
    {
        Destroy();

        device = other.device;
        commandPool = other.commandPool;

        other.device = nullptr;
        other.commandPool = nullptr;

        return *this;
    }

    CommandPool::~CommandPool()
    {
        Destroy();
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

    void CommandPool::Destroy()
    {
        if (device)
        {
            device->DestroyCommandPool(this);
        }
    }
}
