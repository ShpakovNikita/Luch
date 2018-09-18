#include <Husky/Vulkan/VulkanDeviceBufferView.h>
#include <Husky/Vulkan/VulkanGraphicsDevice.h>

namespace Husky::Vulkan
{
    VulkanDeviceBufferView::VulkanDeviceBufferView(
        VulkanGraphicsDevice* aDevice,
        vk::BufferView aBufferView)
        : device(aDevice)
        , bufferView(aBufferView)
    {
    }

    VulkanDeviceBufferView::~VulkanDeviceBufferView()
    {
        Destroy();
    }

    void VulkanDeviceBufferView::Destroy()
    {
        if (device)
        {
            device->DestroyBufferView(this);
        }
    }
}
