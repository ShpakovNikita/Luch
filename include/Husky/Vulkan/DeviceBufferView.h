#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Forwards.h>

namespace Husky::Vulkan
{
    class DeviceBufferView : public BaseObject
    {
        friend class GraphicsDevice;
    public:
        DeviceBufferView(GraphicsDevice* device, vk::BufferView bufferView);

        ~DeviceBufferView() override;

        inline vk::BufferView GetBufferView() { return bufferView; }
    private:
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::BufferView bufferView;
    };
}
