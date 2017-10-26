#pragma once

#include <Husky/Vulkan.h>
#include <Husky/Vulkan/RenderPass.h>
#include <Husky/Vulkan/Framebuffer.h>
#include <Husky/Vulkan/GraphicsDevice.h>
#include <Husky/Vulkan/Pipeline.h>
#include <Husky/Vulkan/VertexBuffer.h>
#include <Husky/Vulkan/IndexBuffer.h>
#include <Husky/Vulkan/IndexType.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;
    class CommandPool;

    class CommandBuffer
    {
        friend class GraphicsDevice;
        friend class CommandPool;
    public:
        CommandBuffer() = default;
        CommandBuffer(GraphicsDevice* device, vk::CommandBuffer commandBuffer);

        CommandBuffer(CommandBuffer&& other);
        CommandBuffer& operator=(CommandBuffer&& other);

        inline vk::CommandBuffer GetCommandBuffer() { return commandBuffer; }

        inline CommandBuffer& Begin()
        {
            vk::CommandBufferBeginInfo beginInfo;
            commandBuffer.begin(beginInfo);
            return *this;
        }

        inline CommandBuffer& End()
        {
            commandBuffer.end();
            return *this;
        }

        // TODO
        //inline CommandBuffer& BeginRenderPass(RenderPass* renderPass, Framebuffer* framebuffer, CommandBuffer* secondaryCommandBuffer);
        
        inline CommandBuffer& BeginInlineRenderPass(RenderPass* renderPass, Framebuffer* framebuffer, const Vector<vk::ClearValue>& clearValues, vk::Rect2D renderArea)
        {
            vk::RenderPassBeginInfo beginInfo;
            beginInfo.setRenderPass(renderPass->GetRenderPass());
            beginInfo.setFramebuffer(framebuffer->GetFramebuffer());
            beginInfo.setClearValueCount((int32)clearValues.size());
            beginInfo.setPClearValues(clearValues.data());
            beginInfo.setRenderArea(renderArea);
            commandBuffer.beginRenderPass(beginInfo, vk::SubpassContents::eInline);
            return *this;
        }

        inline CommandBuffer& BindGraphicsPipeline(Pipeline* pipeline)
        {
            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->GetPipeline());
            return *this;
        }

        inline CommandBuffer& BindVertexBuffers(const Vector<VertexBuffer*>& vertexBuffers, const Vector<int64>& offsets, int32 binding)
        {
            Vector<vk::Buffer> vulkanBuffers;
            vulkanBuffers.reserve(vertexBuffers.size());

            for (auto& vertexBuffer : vertexBuffers)
            {
                vulkanBuffers.push_back(vertexBuffer->GetUnderlyingBuffer().GetBuffer());
            }

            Vector<vk::DeviceSize> vulkanOffsets;
            vulkanOffsets.reserve(offsets.size());

            for (auto& offset : offsets)
            {
                vulkanOffsets.push_back(offset);
            }

            commandBuffer.bindVertexBuffers(binding, vulkanBuffers, vulkanOffsets);
            return *this;
        }

        inline CommandBuffer& BindIndexBuffer(IndexBuffer* indexBuffer, int64 offset)
        {
            commandBuffer.bindIndexBuffer(indexBuffer->GetUnderlyingBuffer().GetBuffer(), offset, ToVulkanIndexType(indexBuffer->GetIndexType()));
            return *this;
        }

        //inline CommandBuffer& NextSubpass(CommandBuffer* secondaryCommandBuffer);

        inline CommandBuffer& NextInlineSubpass()
        {
            commandBuffer.nextSubpass(vk::SubpassContents::eInline);
            return *this;
        }

        inline CommandBuffer& EndRenderPass()
        {
            commandBuffer.endRenderPass();
            return *this;
        }

        inline CommandBuffer& DrawIndexed(int32 indexCount, int32 instanceCount, int32 firstIndex, int32 vertexOffset, int32 firstInstance)
        {
            commandBuffer.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
            return *this;
        }
    private:
        GraphicsDevice* device = nullptr;
        vk::CommandBuffer commandBuffer;
    };
}