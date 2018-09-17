#include <Husky/Vulkan/DeviceBufferView.h>
#include <Husky/Vulkan/GraphicsDevice.h>

namespace Husky::Vulkan
{
    DeviceBufferView::DeviceBufferView(
        GraphicsDevice* aDevice,
        vk::BufferView aBufferView)
        : device(aDevice)
        , bufferView(aBufferView)
    {
    }

    DeviceBufferView::~DeviceBufferView()
    {
        Destroy();
    }

    void DeviceBufferView::Destroy()
    {
        if (device)
        {
            device->DestroyBufferView(this);
        }
    }
}
