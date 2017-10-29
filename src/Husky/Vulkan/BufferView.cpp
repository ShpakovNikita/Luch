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

    BufferView::BufferView(BufferView&& other)
        : device(other.device)
        , bufferView(other.bufferView)
    {
        other.device = nullptr;
        other.bufferView = nullptr;
    }

    BufferView& BufferView::operator=(BufferView&& other)
    {
        Destroy();

        device = other.device;
        bufferView = other.bufferView;

        other.device = nullptr;
        other.bufferView = nullptr;

        return *this;
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