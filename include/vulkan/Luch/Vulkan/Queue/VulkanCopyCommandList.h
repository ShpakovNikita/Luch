#pragma once

#include <Luch/ArrayProxy.h>
#include <Luch/Vulkan.h>
#include <Luch/Graphics/ShaderStage.h>
#include <Luch/Vulkan/DescriptorSet/VulkanDescriptorSet.h>
#include <Luch/Vulkan/Framebuffer/VulkanFramebuffer.h>
#include <Luch/Vulkan/Shader/VulkanShaderStage.h>
#include <Luch/Vulkan/VulkanGraphicsDevice.h>
#include <Luch/Vulkan/Common/VulkanIndexType.h>
#include <Luch/Vulkan/Pipeline/VulkanPipeline.h>
#include <Luch/Vulkan/Pipeline/VulkanPipelineLayout.h>
#include <Luch/Vulkan/RenderPass/VulkanRenderPass.h>
#include <Luch/Vulkan/Pipeline/VulkanPipelineBarrier.h>
#include <Luch/Vulkan/Image/VulkanImage.h>
#include <Luch/Vulkan/Common/VulkanImageAspects.h>
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
