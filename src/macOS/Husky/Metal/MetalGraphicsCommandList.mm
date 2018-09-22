#include <Husky/Metal/MetalGraphicsCommandList.h>
#include <Husky/Metal/MetalGraphicsDevice.h>
#include <Husky/Metal/MetalCommandQueue.h>
#include <Husky/Metal/MetalPipelineState.h>
#include <Husky/Metal/MetalDescriptorSet.h>

using namespace Husky::Graphics;

namespace Husky::Metal
{
    MetalGraphicsCommandList::MetalGraphicsCommandList(
        MetalCommandQueue* queue,
        mtlpp::CommandBuffer commandBuffer)
        : GraphicsCommandList(queue->GetGraphicsDevice())
    {
    }

    void MetalGraphicsCommandList::Begin(const RenderPassCreateInfo& renderPassCreateInfo)
    {
        mtlpp::RenderPassDescriptor renderPassDescriptor;
        commandEncoder = commandBuffer.RenderCommandEncoder(renderPassDescriptor);
    }

    void MetalGraphicsCommandList::End()
    {
        commandEncoder.EndEncoding();
    }

    void MetalGraphicsCommandList::BindPipelineState(PipelineState* pipelineState)
    {
        auto mtlPipelineState = static_cast<MetalPipelineState*>(pipelineState);
        commandEncoder.SetRenderPipelineState(mtlPipelineState->state);
    }

    void MetalGraphicsCommandList::BindTextureDescriptorSet(ShaderStage stage, DescriptorSet* descriptorSet)
    {
        auto mtlDescriptorSet = static_cast<MetalDescriptorSet*>(descriptorSet);

        HUSKY_ASSERT(mtlDescriptorSet->type == DescriptorSetType::Texture);
        auto textures = mtlDescriptorSet->textures.data();
        auto range = ns::Range { (uint32)mtlDescriptorSet->start, (uint32)mtlDescriptorSet->textures.size() };

        switch(stage)
        {
        case ShaderStage::Vertex:
            commandEncoder.SetVertexTextures(textures, range);
            break;
        case ShaderStage::Fragment:
            commandEncoder.SetFragmentTextures(textures, range);
            break;
        default:
            HUSKY_ASSERT(false);
        }
    }

    void MetalGraphicsCommandList::BindBufferDescriptorSet(ShaderStage stage, DescriptorSet* descriptorSet)
    {
        auto mtlDescriptorSet = static_cast<MetalDescriptorSet*>(descriptorSet);

        HUSKY_ASSERT(mtlDescriptorSet->type == DescriptorSetType::Buffer);
        HUSKY_ASSERT(mtlDescriptorSet->buffers.size() == mtlDescriptorSet->bufferOffsets.size());
        auto buffers = mtlDescriptorSet->buffers.data();
        auto bufferOffsets = (uint32*)mtlDescriptorSet->bufferOffsets.data();
        auto range = ns::Range { (uint32)mtlDescriptorSet->start, (uint32)mtlDescriptorSet->buffers.size() };

        switch(stage)
        {
        case ShaderStage::Vertex:
            commandEncoder.SetVertexBuffers(buffers, bufferOffsets, range);
            break;
        case ShaderStage::Fragment:
            commandEncoder.SetFragmentBuffers(buffers, bufferOffsets, range);
            break;
        default:
            HUSKY_ASSERT(false);
        }
    }

    void MetalGraphicsCommandList::BindSamplerDescriptorSet(ShaderStage stage, DescriptorSet* descriptorSet)
    {
        auto mtlDescriptorSet = static_cast<MetalDescriptorSet*>(descriptorSet);

        HUSKY_ASSERT(mtlDescriptorSet->type == DescriptorSetType::Sampler);
        auto samplers = mtlDescriptorSet->samplers.data();
        auto range = ns::Range { (uint32)mtlDescriptorSet->start, (uint32)mtlDescriptorSet->samplers.size() };

        switch(stage)
        {
        case ShaderStage::Vertex:
            commandEncoder.SetVertexSamplerStates(samplers, range);
            break;
        case ShaderStage::Fragment:
            commandEncoder.SetFragmentSamplerStates(samplers, range);
            break;
        default:
            HUSKY_ASSERT(false);
        }
    }

    void MetalGraphicsCommandList::BindVertexBuffers(
        const ArrayProxy<Buffer*> & buffers,
        const ArrayProxy<int32> offsets,
        int32 firstBuffer)
    {

    }
}
