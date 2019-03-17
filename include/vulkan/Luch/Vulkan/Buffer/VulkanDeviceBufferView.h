#pragma once

#include <Luch/BaseObject.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/Common/VulkanForwards.h>

namespace Luch::Vulkan
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
