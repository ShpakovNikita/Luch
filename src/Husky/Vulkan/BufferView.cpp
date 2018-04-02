#include <Husky/Vulkan/BufferView.h>
#include <Husky/Vulkan/GraphicsDevice.h>

namespace Husky::Vulkan
{
    BufferView::BufferView(
        GraphicsDevice* aDevice,
        vk::BufferView aBufferView)
        : device(aDevice)
        , bufferView(aBufferView)
    {
    }

    BufferView::~BufferView()
    {
        Destroy();
    }

    void BufferView::Destroy()
    {
        if (device)
        {
            device->DestroyBufferView(this);
        }
    }
}
