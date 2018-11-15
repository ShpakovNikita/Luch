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
#include <Luch/Vulkan/VulkanPipelineBarrier.h>
#include <Luch/Vulkan/VulkanImage.h>
#include <Luch/Vulkan/VulkanImageAspects.h>

namespace Luch::Vulkan
{
    struct BufferToImageCopy
    {
        int32 bufferOffset = 0;
        int32 bufferRowLength = 0;
        int32 bufferImageHeight = 0;
        vk::Offset3D imageOffset;
        vk::Extent3D imageExtent;
    };

    class VulkanCommandBuffer : public BaseObject
    {
        friend class VulkanGraphicsDevice;
        friend class VulkanCommandPool;
    public:
        static constexpr int32 MaxVertexBuffers = 7;

        VulkanCommandBuffer(VulkanGraphicsDevice* device, vk::CommandBuffer commandBuffer);

        inline vk::CommandBuffer GetCommandBuffer() { return commandBuffer; }

        inline VulkanCommandBuffer* Begin()
        {
            vk::CommandBufferBeginInfo beginInfo;
            commandBuffer.begin(beginInfo);
            return this;
        }

        inline VulkanCommandBuffer* End()
        {
            commandBuffer.end();
            return this;
        }

        // TODO
        //inline CommandBuffer& BeginRenderPass(RenderPass* renderPass, Framebuffer* framebuffer, CommandBuffer* secondaryCommandBuffer);
        
        inline VulkanCommandBuffer* BeginInlineRenderPass(
            VulkanRenderPass* renderPass,
            VulkanFramebuffer* framebuffer,
            Vector<vk::ClearValue> clearValues,
            vk::Rect2D renderArea)
        {
            vk::RenderPassBeginInfo beginInfo;
            beginInfo.setRenderPass(renderPass->GetRenderPass());
            beginInfo.setFramebuffer(framebuffer->GetFramebuffer());
            beginInfo.setClearValueCount(clearValues.size());
            beginInfo.setPClearValues(clearValues.data());
            beginInfo.setRenderArea(renderArea);
            commandBuffer.beginRenderPass(beginInfo, vk::SubpassContents::eInline);
            return this;
        }

        inline VulkanCommandBuffer* BindGraphicsPipeline(VulkanPipeline* pipeline)
        {
            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->GetPipeline());
            return this;
        }

        inline VulkanCommandBuffer* BindVertexBuffers(
            Vector<VulkanDeviceBuffer*> vertexBuffers,
            Vector<int64> offsets,
            int32 binding)
        {
            LUCH_ASSERT(vertexBuffers.size() == offsets.size());

            Vector<vk::Buffer> vulkanBuffers;
            Vector<vk::DeviceSize> vulkanOffsets;

            vulkanBuffers.reserve(vertexBuffers.size());
            vulkanOffsets.reserve(offsets.size());

            for (int32 i = 0; i < vertexBuffers.size(); i++)
            {
                vulkanBuffers.push_back(vertexBuffers[i]->GetBuffer());
                vulkanOffsets.push_back(offsets[i]);
            }

            commandBuffer.bindVertexBuffers(binding, vulkanBuffers, vulkanOffsets);
            return this;
        }

        inline VulkanCommandBuffer* BindIndexBuffer(
            VulkanDeviceBuffer* indexBuffer,
            vk::IndexType indexType,
            int64 offset)
        {
            commandBuffer.bindIndexBuffer(indexBuffer->GetBuffer(), offset, indexType);
            return this;
        }

        inline CommandBuffer* BindDescriptorSets(
            VulkanPipelineLayout* layout,
            int32 firstSet,
            Vector<VulkanDescriptorSet*> sets)
        {
            Vector<vk::DescriptorSet> vulkanSets;
            vulkanSets.reserve(sets.size());

            for (auto& set : sets)
            {
                vulkanSets.push_back(set->GetDescriptorSet());
            }

            commandBuffer.bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics,
                layout->GetPipelineLayout(),
                firstSet,
                vulkanSets,
                {});

            return this;
        }

        inline VulkanCommandBuffer* BindDescriptorSet(
            VulkanPipelineLayout* layout,
            int32 index,
            VulkanDescriptorSet* set)
        {
            commandBuffer.bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics,
                layout->GetPipelineLayout(),
                index,
                { set->GetDescriptorSet() },
                {});

            return this;
        }

        inline VulkanCommandBuffer* NextInlineSubpass()
        {
            commandBuffer.nextSubpass(vk::SubpassContents::eInline);
            return this;
        }

        inline VulkanCommandBuffer* EndRenderPass()
        {
            commandBuffer.endRenderPass();
            return this;
        }

        inline VulkanCommandBuffer* DrawIndexed(
            int32 indexCount,
            int32 instanceCount,
            int32 firstIndex,
            int32 vertexOffset,
            int32 firstInstance)
        {
            commandBuffer.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
            return this;
        }

        inline VulkanCommandBuffer* Draw(
            int32 vertexCount,
            int32 instanceCount,
            int32 firstVertex,
            int32 firstInstance)
        {
            commandBuffer.draw(vertexCount, instanceCount, firstVertex, firstInstance);
            return this;
        }

        inline VulkanCommandBuffer* PipelineBarrier(
            const PipelineBarrier& barrier)
        {
            auto vulkanBarrier = PipelineBarrier::ToVulkanPipelineBarrier(barrier);

            commandBuffer.pipelineBarrier(
                vulkanBarrier.srcStageMask,
                vulkanBarrier.dstStageMask,
                vulkanBarrier.dependencyFlags,
                {},
                vulkanBarrier.bufferMemoryBarriers,
                vulkanBarrier.imageMemoryBarriers
            );

            return this;
        }

        inline VulkanCommandBuffer* CopyBufferToImage(
            VulkanDeviceBuffer* buffer,
            VulkanImage* image,
            vk::ImageLayout dstLayout,
            const BufferToImageCopy& copy)
        {
            vk::ImageSubresourceLayers subresource;
            subresource.setAspectMask(ToVulkanImageAspectFlags(image->GetImageAspects()));
            subresource.setBaseArrayLayer(0);
            subresource.setLayerCount(1);
            subresource.setMipLevel(0);

            vk::BufferImageCopy bufferImageCopy;
            bufferImageCopy.setBufferOffset(copy.bufferOffset);
            bufferImageCopy.setBufferRowLength(copy.bufferRowLength);
            bufferImageCopy.setBufferImageHeight(copy.bufferImageHeight);
            bufferImageCopy.setImageOffset(copy.imageOffset);
            bufferImageCopy.setImageExtent(copy.imageExtent);
            bufferImageCopy.setImageSubresource(subresource);

            commandBuffer.copyBufferToImage(
                buffer->GetBuffer(),
                image->GetImage(),
                dstLayout,
                { bufferImageCopy }
            );

            return this;
        }

        inline VulkanCommandBuffer* SetViewport(const vk::Viewport& viewport)
        {
            commandBuffer.setViewport(0, { viewport });
            return this;
        }

        inline VulkanCommandBuffer* SetScissor(const vk::Rect2D& scissor)
        {
            commandBuffer.setScissor(0, { scissor });
            return this;
        }

        template<typename T>
        inline VulkanCommandBuffer* PushConstants(
            VulkanPipelineLayout* layout,
            ShaderStage stages,
            int32 offset,
            const T& value)
        {
            commandBuffer.pushConstants(
                layout->GetPipelineLayout(),
                ToVulkanShaderStages(stages),
                offset,
                sizeof(T),
                &value);

            return this;
        }
    private:
        VulkanGraphicsDevice* device = nullptr;
        vk::CommandBuffer commandBuffer;
    };
}
