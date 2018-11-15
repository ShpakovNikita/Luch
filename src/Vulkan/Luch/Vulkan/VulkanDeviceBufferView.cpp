#include <Luch/Vulkan/VulkanDeviceBufferView.h>
#include <Luch/Vulkan/VulkanGraphicsDevice.h>

namespace Luch::Vulkan
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
