#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Forwards.h>

namespace Husky::Vulkan
{
    class VulkanDeviceBufferView : public BaseObject
    {
        friend class VulkanGraphicsDevice;
    public:
        VulkanDeviceBufferView(
            VulkanGraphicsDevice* device,
            vk::BufferView bufferView);

        ~VulkanDeviceBufferView() override;

        inline vk::BufferView GetBufferView() { return bufferView; }
    private:
        void Destroy();

        VulkanGraphicsDevice* device = nullptr;
        vk::BufferView bufferView;
    };
}
