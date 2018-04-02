#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{
    class BufferView : public BaseObject
    {
        friend class GraphicsDevice;
    public:
        BufferView(GraphicsDevice* device, vk::BufferView bufferView);

        BufferView(BufferView&& other) = delete;
        BufferView(const BufferView& other) = delete;
        BufferView& operator=(const BufferView& other) = delete;
        BufferView& operator=(BufferView&& other) = delete;

        ~BufferView() override;

        inline vk::BufferView GetBufferView() { return bufferView; }
    private:
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::BufferView bufferView;
    };
}
