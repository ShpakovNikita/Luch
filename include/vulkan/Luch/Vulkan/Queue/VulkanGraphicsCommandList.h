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

        void BeginRenderPass(FrameBuffer* framebuffer) override;
        void EndRenderPass() override;

        void BindPipelineState(PipelineState* pipelineState) override;

        void BindTextureDescriptorSet(
            ShaderStage stage,
            PipelineLayout* pipelineLayout,
            DescriptorSet* descriptorSet) override;

        void BindBufferDescriptorSet(
            ShaderStage stage,
            PipelineLayout* pipelineLayout,
            DescriptorSet* descriptorSet) override;

        void BindSamplerDescriptorSet(
            ShaderStage stage,
            PipelineLayout* pipelineLayout,
            DescriptorSet* descriptorSet) override;

        void BindVertexBuffers(
            const ArrayProxy<Buffer*>& buffers,
            const ArrayProxy<int32>& offsets) override;

        void BindIndexBuffer(
            Buffer* indexBuffer,
            IndexType indexType,
            int32 indexBufferOffset) override;

        void SetViewports(const ArrayProxy<Viewport>& viewports) override;
        void SetScissorRects(const ArrayProxy<Rect2i>& scissorRects) override;

        void Draw(
            int32 vertexStart,
            int32 vertexCount) override;

        void DrawInstanced(
            int32 vertexStart,
            int32 vertexCount,
            int32 instanceCount,
            int32 baseInstance) override;

        void DrawIndexedInstanced(
            int32 indexCount,
            int32 baseVertex,
            int32 instanceCount,
            int32 baseInstance) override;

        inline vk::CommandBuffer GetCommandBuffer() { return commandBuffer; }

        /*

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
         */

    private:
        VulkanGraphicsDevice* device = nullptr;
        vk::CommandBuffer commandBuffer;
    };
}
