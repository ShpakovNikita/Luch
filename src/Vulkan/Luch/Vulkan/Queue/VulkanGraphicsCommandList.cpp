#include <Luch/Vulkan/Queue/VulkanGraphicsCommandList.h>

namespace Luch::Vulkan
{
    VulkanGraphicsCommandList::VulkanGraphicsCommandList(
        VulkanGraphicsDevice* aDevice,
        vk::CommandBuffer aCommandBuffer)
        : GraphicsCommandList(aDevice)
        , device(aDevice)
        , commandBuffer(aCommandBuffer)
    {
    }

    void VulkanGraphicsCommandList::Begin()
    {
        vk::CommandBufferBeginInfo beginInfo;
        commandBuffer.begin(beginInfo);
    }

    void VulkanGraphicsCommandList::End()
    {
        commandBuffer.end();
    }

    void VulkanGraphicsCommandList::BeginRenderPass(FrameBuffer* framebuffer)
    {
        VulkanRenderPass* renderPass;
        VulkanFramebuffer* framebufferz;
        Vector<vk::ClearValue> clearValues;
        vk::Rect2D renderArea;
        //
//        vk::RenderPassBeginInfo beginInfo;
//        beginInfo.setRenderPass(renderPass->GetRenderPass());
//        beginInfo.setFramebuffer(framebuffer->GetFramebuffer());
//        beginInfo.setClearValueCount(clearValues.size());
//        beginInfo.setPClearValues(clearValues.data());
//        beginInfo.setRenderArea(renderArea);
//        commandBuffer.beginRenderPass(beginInfo, vk::SubpassContents::eInline);
    }

    void VulkanGraphicsCommandList::EndRenderPass()
    {
        commandBuffer.endRenderPass();
    }

    void VulkanGraphicsCommandList::BindGraphicsPipelineState(GraphicsPipelineState* pipelineState)
    {
        // commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->GetPipeline());
    }

    void VulkanGraphicsCommandList::BindTiledPipelineState(TiledPipelineState* pipelineState)
    {

    }

    void VulkanGraphicsCommandList::BindTextureDescriptorSet(
        ShaderStage stage,
        PipelineLayout* pipelineLayout,
        DescriptorSet* descriptorSet)
    {
//        commandBuffer.bindDescriptorSets(
//            vk::PipelineBindPoint::eGraphics,
//            layout->GetPipelineLayout(),
//            index,
//            { set->GetDescriptorSet() },
//            {});
    }

    void VulkanGraphicsCommandList::BindBufferDescriptorSet(
        ShaderStage stage,
        PipelineLayout* pipelineLayout,
        DescriptorSet* descriptorSet)
    {
    }

    void VulkanGraphicsCommandList::BindSamplerDescriptorSet(
        ShaderStage stage,
        PipelineLayout* pipelineLayout,
        DescriptorSet* descriptorSet)
    {
    }

    void VulkanGraphicsCommandList::BindVertexBuffers(
        const ArrayProxy<Buffer*>& buffers,
        const ArrayProxy<int32>& offsets)
    {
        LUCH_ASSERT(buffers.size() == offsets.size());

        Vector<vk::Buffer> vulkanBuffers;
        Vector<vk::DeviceSize> vulkanOffsets;

        vulkanBuffers.reserve(buffers.size());
        vulkanOffsets.reserve(offsets.size());

        for (int32 i = 0; i < buffers.size(); i++)
        {
            // vulkanBuffers.push_back(buffers.data()[i]->GetBuffer());
            vulkanOffsets.push_back(offsets.data()[i]);
        }

        //commandBuffer.bindVertexBuffers(binding, vulkanBuffers, vulkanOffsets);
    }

    void VulkanGraphicsCommandList::BindIndexBuffer(
        Buffer* indexBuffer,
        IndexType indexType,
        int32 indexBufferOffset)
    {
        // commandBuffer.bindIndexBuffer(indexBuffer->GetBuffer(), indexBufferOffset, indexType);
    }

    void VulkanGraphicsCommandList::SetViewports(const ArrayProxy<Viewport>& viewports)
    {
        // commandBuffer.setViewport(0, { viewport });
    }

    void VulkanGraphicsCommandList::SetScissorRects(const ArrayProxy<Rect2i>& scissorRects)
    {
        // commandBuffer.setScissor(0, { scissor });
    }

    void VulkanGraphicsCommandList::Draw(int32 vertexStart, int32 vertexCount)
    {
        commandBuffer.draw(vertexCount, 1, vertexStart, 0);
    }

    void VulkanGraphicsCommandList::DrawInstanced(
        int32 vertexStart,
        int32 vertexCount,
        int32 instanceCount,
        int32 baseInstance)
    {
        commandBuffer.draw(vertexCount, instanceCount, vertexStart, baseInstance);
    }

    void VulkanGraphicsCommandList::DrawIndexedInstanced(
        int32 indexCount,
        int32 baseVertex,
        int32 instanceCount,
        int32 baseInstance)
    {
        // missing info?
        // commandBuffer.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    void VulkanGraphicsCommandList::SetLabel(const String& label)
    {
        // todo: implement
    }
}
