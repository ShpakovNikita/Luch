#pragma once

#include <Luch/ArrayProxy.h>
#include <Luch/Vulkan.h>
#include <Luch/Graphics/ShaderStage.h>
#include <Luch/Vulkan/VulkanDeviceBuffer.h>
#include <Luch/Vulkan/VulkanDescriptorSet.h>
#include <Luch/Vulkan/VulkanFramebuffer.h>
#include <Luch/Vulkan/VulkanShaderStage.h>
#include <Luch/Vulkan/VulkanGraphicsDevice.h>
#include <Luch/Vulkan/VulkanIndexType.h>
#include <Luch/Vulkan/VulkanPipeline.h>
#include <Luch/Vulkan/VulkanPipelineLayout.h>
#include <Luch/Vulkan/VulkanRenderPass.h>
#include <Luch/Vulkan/PipelineBarrier.h>
#include <Luch/Vulkan/VulkanImage.h>
#include <Luch/Vulkan/VulkanImageAspects.h>
#include <Luch/Graphics/CopyCommandList.h>

// todo: remove unneeded headers

using namespace Luch::Graphics;

namespace Luch::Vulkan
{
    class VulkanCopyCommandList : public CopyCommandList
    {
        friend class VulkanGraphicsDevice;
        friend class VulkanCommandPool;
    public:
        VulkanCopyCommandList(VulkanGraphicsDevice* device, vk::CommandBuffer commandBuffer);

        void Begin() override;
        void End() override;

        void CopyBufferToTexture(Buffer* buffer, Texture* texture, const BufferToTextureCopy& copy) override;

        inline vk::CommandBuffer GetCommandBuffer() { return commandBuffer; }

    private:
        VulkanGraphicsDevice* device = nullptr;
        vk::CommandBuffer commandBuffer;
    };
}
