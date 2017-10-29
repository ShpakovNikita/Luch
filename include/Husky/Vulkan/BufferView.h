#pragma once

#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{
    class BufferView
    {
        friend class GraphicsDevice;
    public:
        BufferView() = default;

        BufferView(BufferView&& other);
        BufferView& operator=(BufferView&& other);

        ~BufferView();

        inline vk::BufferView GetBufferView() { return bufferView; }
    private:
        BufferView(GraphicsDevice* device, vk::BufferView bufferView);
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::BufferView bufferView;
    };
}
