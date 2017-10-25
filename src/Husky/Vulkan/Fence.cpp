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

    Fence& Fence::operator=(Fence && other)
    {
        Destroy();

        device = other.device;
        fence = other.fence;

        other.device = nullptr;
        other.fence = nullptr;

        return *this;
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
