#pragma once

#include <Husky/ArrayProxy.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Buffer.h>
#include <Husky/Vulkan/DescriptorSet.h>
#include <Husky/Vulkan/Framebuffer.h>
#include <Husky/Vulkan/GraphicsDevice.h>
#include <Husky/Vulkan/IndexType.h>
#include <Husky/Vulkan/Pipeline.h>
#include <Husky/Vulkan/PipelineLayout.h>
#include <Husky/Vulkan/RenderPass.h>
#include <Husky/Vulkan/PipelineBarrier.h>
#include <Husky/Vulkan/Image.h>
#include <Husky/Vulkan/ImageAspects.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;
    class CommandPool;

    struct BufferToImageCopy
    {
        int32 bufferOffset = 0;
        int32 bufferRowLength = 0;
        int32 bufferImageHeight = 0;
        vk::Offset3D imageOffset;
        vk::Extent3D imageExtent;
    };

    class CommandBuffer : public BaseObject
    {
        friend class GraphicsDevice;
        friend class CommandPool;
    public:
        CommandBuffer(GraphicsDevice* device, vk::CommandBuffer commandBuffer);

        CommandBuffer(CommandBuffer&& other) = delete;
        CommandBuffer(const CommandBuffer& other) = delete;
        CommandBuffer& operator=(const CommandBuffer& other) = delete;
        CommandBuffer& operator=(CommandBuffer&& other) = delete;

        inline vk::CommandBuffer GetCommandBuffer() { return commandBuffer; }

        inline CommandBuffer* Begin()
        {
            vk::CommandBufferBeginInfo beginInfo;
            commandBuffer.begin(beginInfo);
            return this;
        }

        inline CommandBuffer* End()
        {
            commandBuffer.end();
            return this;
        }

        // TODO
        //inline CommandBuffer& BeginRenderPass(RenderPass* renderPass, Framebuffer* framebuffer, CommandBuffer* secondaryCommandBuffer);
        
        inline CommandBuffer* BeginInlineRenderPass(RenderPass* renderPass, Framebuffer* framebuffer, Vector<vk::ClearValue> clearValues, vk::Rect2D renderArea)
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

        inline CommandBuffer* BindGraphicsPipeline(Pipeline* pipeline)
        {
            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->GetPipeline());
            return this;
        }

        inline CommandBuffer* BindVertexBuffers(Vector<Buffer*> vertexBuffers, Vector<int64> offsets, int32 binding)
        {
            Vector<vk::Buffer> vulkanBuffers;
            vulkanBuffers.reserve(vertexBuffers.size());

            for (auto& vertexBuffer : vertexBuffers)
            {
                vulkanBuffers.push_back(vertexBuffer->GetBuffer());
            }

            Vector<vk::DeviceSize> vulkanOffsets;
            vulkanOffsets.reserve(offsets.size());

            for (auto& offset : offsets)
            {
                vulkanOffsets.push_back(offset);
            }

            commandBuffer.bindVertexBuffers(binding, vulkanBuffers, vulkanOffsets);
            return this;
        }

        inline CommandBuffer* BindIndexBuffer(Buffer* indexBuffer, vk::IndexType indexType, int64 offset)
        {
            commandBuffer.bindIndexBuffer(indexBuffer->GetBuffer(), offset, indexType);
            return this;
        }

        inline CommandBuffer* BindDescriptorSets(PipelineLayout* layout, int32 firstSet, Vector<DescriptorSet*> sets)
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

        inline CommandBuffer* BindDescriptorSet(PipelineLayout* layout, int32 index, DescriptorSet* set)
        {
            commandBuffer.bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics,
                layout->GetPipelineLayout(),
                index,
                { set->GetDescriptorSet() },
                {});

            return this;
        }

        //inline CommandBuffer& NextSubpass(CommandBuffer* secondaryCommandBuffer);

        inline CommandBuffer* NextInlineSubpass()
        {
            commandBuffer.nextSubpass(vk::SubpassContents::eInline);
            return this;
        }

        inline CommandBuffer* EndRenderPass()
        {
            commandBuffer.endRenderPass();
            return this;
        }

        inline CommandBuffer* DrawIndexed(int32 indexCount, int32 instanceCount, int32 firstIndex, int32 vertexOffset, int32 firstInstance)
        {
            commandBuffer.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
            return this;
        }

        inline CommandBuffer* PipelineBarrier(const PipelineBarrier& barrier)
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

        inline CommandBuffer* CopyBufferToImage(
            Buffer* buffer,
            Image* image,
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

        inline CommandBuffer* SetViewport(const vk::Viewport& viewport)
        {
            commandBuffer.setViewport(0, { viewport });
            return this;
        }

        inline CommandBuffer* SetScissor(const vk::Rect2D& scissor)
        {
            commandBuffer.setScissor(0, { scissor });
            return this;
        }
    private:
        GraphicsDevice* device = nullptr;
        vk::CommandBuffer commandBuffer;
    };
}