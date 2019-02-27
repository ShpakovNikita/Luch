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
#include <Luch/Graphics/GraphicsCommandList.h>

using namespace Luch::Graphics;

namespace Luch::Vulkan
{
    class VulkanGraphicsCommandList : public GraphicsCommandList
    {
        friend class VulkanGraphicsDevice;
        friend class VulkanCommandPool;
    public:
        VulkanGraphicsCommandList(VulkanGraphicsDevice* device, vk::CommandBuffer commandBuffer);

        void Begin() override;
        void End() override;

        virtual void BeginRenderPass(FrameBuffer* framebuffer) override {}
        virtual void EndRenderPass() override {}

        virtual void BindPipelineState(PipelineState* pipelineState) override {}

        virtual void BindTextureDescriptorSet(
            ShaderStage stage,
            PipelineLayout* pipelineLayout,
            DescriptorSet* descriptorSet) override {}

        virtual void BindBufferDescriptorSet(
            ShaderStage stage,
            PipelineLayout* pipelineLayout,
            DescriptorSet* descriptorSet) override {}

        virtual void BindSamplerDescriptorSet(
            ShaderStage stage,
            PipelineLayout* pipelineLayout,
            DescriptorSet* descriptorSet) override {}

        virtual void BindVertexBuffers(
            const ArrayProxy<Buffer*>& buffers,
            const ArrayProxy<int32>& offsets) override {}

        virtual void BindIndexBuffer(
            Buffer* indexBuffer,
            IndexType indexType,
            int32 indexBufferOffset) override {}

        virtual void SetViewports(const ArrayProxy<Viewport>& viewports) override {}
        virtual void SetScissorRects(const ArrayProxy<Rect2i>& scissorRects) override {}

        virtual void Draw(
            int32 vertexStart,
            int32 vertexCount) override {}

        virtual void DrawInstanced(
            int32 vertexStart,
            int32 vertexCount,
            int32 instanceCount,
            int32 baseInstance) override {}

        virtual void DrawIndexedInstanced(
            int32 indexCount,
            int32 baseVertex,
            int32 instanceCount,
            int32 baseInstance) override {}

        inline vk::CommandBuffer GetCommandBuffer() { return commandBuffer; }

    private:
        VulkanGraphicsDevice* device = nullptr;
        vk::CommandBuffer commandBuffer;
    };
}
