#pragma once

#include <Luch/BaseObject.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/Common/VulkanForwards.h>
#include <Luch/Graphics/GraphicsResultValue.h>
#include <Luch/Graphics/Buffer.h>
#include <Luch/Graphics/BufferCreateInfo.h>

using namespace Luch::Graphics;

namespace Luch::Vulkan
{
    class VulkanDeviceBuffer : public Buffer
    {
        friend class VulkanGraphicsDevice;
    public:
        VulkanDeviceBuffer(
            VulkanGraphicsDevice* device,
            vk::Buffer buffer,
            vk::DeviceMemory memory,
            vk::BufferCreateInfo bufferCreateInfo);

        ~VulkanDeviceBuffer() override;

        GraphicsResultValue<void*> MapMemory(int32 size, int32 offset) override;
        void* GetMappedMemory() override { return mappedMemory; }
        GraphicsResult UnmapMemory() override;

        const BufferCreateInfo& GetCreateInfo() const override
        {
            return createInfoUnused;
        }

        // TODO more control
        vk::Result FlushMemory();
        vk::Result InvalidateMemory();

        inline VulkanGraphicsDevice* GetDevice() { return device; }
        inline vk::Buffer GetBuffer() { return buffer; }
        inline vk::DeviceMemory GetDeviceMemory() { return memory; }
    private:
        void Destroy();

        VulkanGraphicsDevice* device = nullptr;
        vk::Buffer buffer;
        vk::DeviceMemory memory;
        vk::BufferCreateInfo bufferCreateInfo;
        BufferCreateInfo createInfoUnused; // todo: fix

        void* mappedMemory = nullptr;
    };
}
