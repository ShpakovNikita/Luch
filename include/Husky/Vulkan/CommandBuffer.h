#pragma once

#include <Husky/Vulkan.h>
#include <Husky/Vulkan/RenderPass.h>
#include <Husky/Vulkan/Framebuffer.h>
#include <Husky/Vulkan/GraphicsDevice.h>
#include <Husky/Vulkan/Pipeline.h>

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

        CommandBuffer& End()
        {
            commandBuffer.end();
            return *this;
        }

        // TODO
        //inline CommandBuffer& BeginRenderPass(RenderPass* renderPass, Framebuffer* framebuffer, CommandBuffer* secondaryCommandBuffer);
        
        inline CommandBuffer& BeginInlineRenderPass(RenderPass* renderPass, Framebuffer* framebuffer)
        {
            vk::RenderPassBeginInfo beginInfo;
            beginInfo.setRenderPass(renderPass->GetRenderPass());
            beginInfo.setFramebuffer(framebuffer->GetFramebuffer());
            commandBuffer.beginRenderPass(beginInfo, vk::SubpassContents::eInline);
            return *this;
        }

        CommandBuffer& BindGraphicsPipeline(Pipeline* pipeline)
        {
            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->GetPipeline());
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