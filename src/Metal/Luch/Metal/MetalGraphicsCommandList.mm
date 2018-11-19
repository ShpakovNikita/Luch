#include <Luch/Metal/MetalGraphicsCommandList.h>
#include <Luch/Graphics/PipelineLayoutCreateInfo.h>
#include <Luch/Metal/MetalGraphicsDevice.h>
#include <Luch/Metal/MetalCommandQueue.h>
#include <Luch/Metal/MetalPipelineState.h>
#include <Luch/Metal/MetalPipelineLayout.h>
#include <Luch/Metal/MetalDescriptorSet.h>
#include <Luch/Metal/MetalRenderPassCreateInfo.h>
#include <Luch/Metal/MetalDescriptorSetLayout.h>
#include <Luch/Metal/MetalBuffer.h>
#include <Luch/Metal/MetalPrimitiveTopology.h>
#include <Luch/Metal/MetalIndexType.h>
#include <Luch/Assert.h>

using namespace Luch::Graphics;

namespace Luch::Metal
{
    mtlpp::CullMode ToMetalCullMode(CullMode cullMode)
    {
        switch(cullMode)
        {
        case CullMode::Back:
            return mtlpp::CullMode::Back;
        case CullMode::Front:
            return mtlpp::CullMode::Front;
        case CullMode::None:
            return mtlpp::CullMode::None;
        default:
            LUCH_ASSERT_MSG(false, "Unknown cull mode");
            return mtlpp::CullMode::None;
        }
    }

    mtlpp::Winding ToMetalWinding(FrontFace frontFace)
    {
        switch(frontFace)
        {
        case FrontFace::Clockwise:
            return mtlpp::Winding::Clockwise;
        case FrontFace::CounterClockwise:
            return mtlpp::Winding::CounterClockwise;
        default:
            LUCH_ASSERT_MSG(false, "Unknown front face winding order");
            return mtlpp::Winding::CounterClockwise;
        }
    }

    mtlpp::TriangleFillMode ToMetalTriangleFillMode(PolygonMode polygonMode)
    {
        switch(polygonMode)
        {
        case PolygonMode::Fill:
            return mtlpp::TriangleFillMode::Fill;
        case PolygonMode::Lines:
            return mtlpp::TriangleFillMode::Lines;
        default:
            LUCH_ASSERT_MSG(false, "Unknown polygon fill mode");
            return mtlpp::TriangleFillMode::Lines;
        }
    }

    MetalGraphicsCommandList::MetalGraphicsCommandList(
        MetalGraphicsDevice* device,
        mtlpp::CommandBuffer aCommandBuffer)
        : GraphicsCommandList(device)
        , commandBuffer(aCommandBuffer)
    {
    }

    void MetalGraphicsCommandList::BeginRenderPass(const RenderPassCreateInfo& renderPassCreateInfo)
    {
        LUCH_ASSERT(!commandEncoder);
        auto renderPassDescriptor = ToMetalRenderPasDescriptor(renderPassCreateInfo);
        commandEncoder = commandBuffer.RenderCommandEncoder(renderPassDescriptor);
    }

    void MetalGraphicsCommandList::EndRenderPass()
    {
        commandEncoder.EndEncoding();
        commandEncoder = { };
    }

    void MetalGraphicsCommandList::BindPipelineState(PipelineState* pipelineState)
    {
        auto mtlPipelineState = static_cast<MetalPipelineState*>(pipelineState);
        const auto& ci = mtlPipelineState->GetCreateInfo();

        primitiveType = ToMetalPrimitiveType(ci.inputAssembler.primitiveTopology);

        commandEncoder.SetRenderPipelineState(mtlPipelineState->pipelineState);

        if(mtlPipelineState->depthStencilState.has_value())
        {
            LUCH_ASSERT(mtlPipelineState->GetCreateInfo().depthStencil.depthTestEnable == true);
            commandEncoder.SetDepthStencilState(mtlPipelineState->depthStencilState.value());
        }
        else
        {
            LUCH_ASSERT(mtlPipelineState->GetCreateInfo().depthStencil.depthTestEnable == false);
        }
        // TODO stencil reference
        commandEncoder.SetCullMode(ToMetalCullMode(ci.rasterization.cullMode));
        commandEncoder.SetFrontFacingWinding(ToMetalWinding(ci.rasterization.frontFace));

        if(ci.rasterization.depthBiasEnable)
        {
            commandEncoder.SetDepthBias(
                ci.rasterization.depthBiasConstantFactor,
                ci.rasterization.depthBiasSlopeFactor,
                ci.rasterization.depthBiasClamp);
        }

        const auto& blendColor = ci.colorAttachments.blendColor;
        commandEncoder.SetBlendColor(
            blendColor.red,
            blendColor.green,
            blendColor.blue,
            blendColor.alpha);

        commandEncoder.SetDepthClipMode(ci.rasterization.depthClampEnable
            ? mtlpp::DepthClipMode::Clamp
            : mtlpp::DepthClipMode::Clip);

        commandEncoder.SetTriangleFillMode(ToMetalTriangleFillMode(ci.rasterization.polygonMode));
    }

    void MetalGraphicsCommandList::BindTextureDescriptorSet(
        ShaderStage stage,
        PipelineLayout* pipelineLayout,
        DescriptorSet* descriptorSet)
    {
        auto mtlDescriptorSet = static_cast<MetalDescriptorSet*>(descriptorSet);
        auto mtlDescriptorSetLayout = mtlDescriptorSet->descriptorSetLayout;
        auto mtlPipelineLayout = static_cast<MetalPipelineLayout*>(pipelineLayout);
        auto textureSetLayouts = mtlPipelineLayout->createInfo.stages[stage].textureSetLayouts;
        auto layoutIt = std::find(textureSetLayouts.begin(), textureSetLayouts.end(), mtlDescriptorSetLayout);
        LUCH_ASSERT(layoutIt != textureSetLayouts.end());

        uint32 start = 0;
        for(auto it = textureSetLayouts.begin(); it != layoutIt; it++)
        {
            auto mtlLayout = static_cast<MetalDescriptorSetLayout*>(*it);
            start += mtlLayout->createInfo.bindings.size();
        }

        LUCH_ASSERT(mtlDescriptorSetLayout->createInfo.type == DescriptorSetType::Texture);
        auto textures = mtlDescriptorSet->textures.data();
        auto length = (uint32)mtlDescriptorSet->textures.size();

        LUCH_ASSERT(length != 0);
        auto range = ns::Range { start, length };

        switch(stage)
        {
        case ShaderStage::Vertex:
            commandEncoder.SetVertexTextures(textures, range);
            break;
        case ShaderStage::Fragment:
            commandEncoder.SetFragmentTextures(textures, range);
            break;
        default:
            LUCH_ASSERT(false);
        }
    }

    void MetalGraphicsCommandList::BindBufferDescriptorSet(
        ShaderStage stage,
        PipelineLayout* pipelineLayout,
        DescriptorSet* descriptorSet)
    {
        auto mtlDescriptorSet = static_cast<MetalDescriptorSet*>(descriptorSet);
        auto mtlDescriptorSetLayout = mtlDescriptorSet->descriptorSetLayout;
        auto mtlPipelineLayout = static_cast<MetalPipelineLayout*>(pipelineLayout);
        auto bufferSetLayouts = mtlPipelineLayout->createInfo.stages[stage].bufferSetLayouts;
        auto layoutIt = std::find(bufferSetLayouts.begin(), bufferSetLayouts.end(), mtlDescriptorSetLayout);
        LUCH_ASSERT(layoutIt != bufferSetLayouts.end());

        uint32 start = 0;
        for(auto it = bufferSetLayouts.begin(); it != layoutIt; it++)
        {
            auto mtlLayout = static_cast<MetalDescriptorSetLayout*>(*it);
            start += mtlLayout->createInfo.bindings.size();
        }

        LUCH_ASSERT(mtlDescriptorSetLayout->createInfo.type == DescriptorSetType::Buffer);
        auto buffers = mtlDescriptorSet->buffers.data();
        auto bufferOffsets = (uint32*)mtlDescriptorSet->bufferOffsets.data();
        auto length = (uint32)mtlDescriptorSet->buffers.size();

        LUCH_ASSERT(length != 0);
        auto range = ns::Range { start, length };

        switch(stage)
        {
        case ShaderStage::Vertex:
            commandEncoder.SetVertexBuffers(buffers, bufferOffsets, range);
            break;
        case ShaderStage::Fragment:
            commandEncoder.SetFragmentBuffers(buffers, bufferOffsets, range);
            break;
        default:
            LUCH_ASSERT(false);
        }
    }

    void MetalGraphicsCommandList::BindSamplerDescriptorSet(
        ShaderStage stage,
        PipelineLayout* pipelineLayout,
        DescriptorSet* descriptorSet)
    {
        auto mtlDescriptorSet = static_cast<MetalDescriptorSet*>(descriptorSet);
        auto mtlDescriptorSetLayout = mtlDescriptorSet->descriptorSetLayout;
        auto mtlPipelineLayout = static_cast<MetalPipelineLayout*>(pipelineLayout);
        auto samplerSetLayouts = mtlPipelineLayout->createInfo.stages[stage].samplerSetLayouts;
        auto layoutIt = std::find(samplerSetLayouts.begin(), samplerSetLayouts.end(), mtlDescriptorSetLayout);
        LUCH_ASSERT(layoutIt != samplerSetLayouts.end());

        uint32 start = 0;
        for(auto it = samplerSetLayouts.begin(); it != layoutIt; it++)
        {
            auto mtlLayout = static_cast<MetalDescriptorSetLayout*>(*it);
            start += mtlLayout->createInfo.bindings.size();
        }

        LUCH_ASSERT(mtlDescriptorSetLayout->createInfo.type == DescriptorSetType::Sampler);
        auto samplers = mtlDescriptorSet->samplers.data();
        auto length = (uint32)mtlDescriptorSet->samplers.size();

        LUCH_ASSERT(length != 0);
        auto range = ns::Range { start, length };

        switch(stage)
        {
        case ShaderStage::Vertex:
            commandEncoder.SetVertexSamplerStates(samplers, range);
            break;
        case ShaderStage::Fragment:
            commandEncoder.SetFragmentSamplerStates(samplers, range);
            break;
        default:
            LUCH_ASSERT(false);
        }
    }

    void MetalGraphicsCommandList::BindVertexBuffers(
        const ArrayProxy<Buffer*>& buffers,
        const ArrayProxy<int32>& offsets,
        int32 firstBuffer)
    {
        LUCH_ASSERT(buffers.size() == offsets.size());
        auto mtlGraphicsDevice = (MetalGraphicsDevice*)GetGraphicsDevice();
        int32 start = mtlGraphicsDevice->MaxBoundBuffers() - 1 - buffers.size();
        ns::Range range { (uint32)start, (uint32)buffers.size() };

        Vector<mtlpp::Buffer> mtlBuffers;
        mtlBuffers.reserve(buffers.size());
        for(auto buffer : buffers)
        {
            auto mtlBuffer = static_cast<MetalBuffer*>(buffer);
            mtlBuffers.push_back(mtlBuffer->buffer);
        }

        commandEncoder.SetVertexBuffers(mtlBuffers.data(), (uint32*)offsets.data(), range);
    }

    void MetalGraphicsCommandList::BindIndexBuffer(
        Buffer* aIndexBuffer,
        IndexType aIndexType,
        int32 aIndexBufferOffset)
    {
        indexBuffer = static_cast<MetalBuffer*>(aIndexBuffer)->buffer;
        indexType = ToMetalIndexType(aIndexType);
        indexBufferOffset = aIndexBufferOffset;
    }

    void MetalGraphicsCommandList::SetViewports(const ArrayProxy<Viewport>& viewports)
    {
        LUCH_ASSERT(viewports.size() == 1);

        auto viewport = viewports.front();
        commandEncoder.SetViewport({
            viewport.x,
            viewport.y,
            viewport.width,
            viewport.height,
            viewport.minDepth,
            viewport.maxDepth});
    }

    void MetalGraphicsCommandList::SetScissorRects(const ArrayProxy<IntRect>& scissorRects)
    {
        LUCH_ASSERT(scissorRects.size() == 1);

        auto scissorRect = scissorRects.front();
        commandEncoder.SetScissorRect({
            (uint32)scissorRect.origin.x,
            (uint32)scissorRect.origin.y,
            (uint32)scissorRect.size.width,
            (uint32)scissorRect.size.height});
    }

    void MetalGraphicsCommandList::Draw(
        int32 vertexStart,
        int32 vertexCount)
    {
        commandEncoder.Draw(primitiveType, (uint32)vertexStart, (uint32)vertexCount);
    }

    void MetalGraphicsCommandList::DrawInstanced(
        int32 vertexStart,
        int32 vertexCount,
        int32 instanceCount,
        int32 baseInstance)
    {
        commandEncoder.Draw(
            primitiveType,
            (uint32)vertexStart,
            (uint32)vertexCount,
            (uint32)instanceCount,
            (uint32)baseInstance);
    }

    void MetalGraphicsCommandList::DrawIndexedInstanced(
        int32 indexCount,
        int32 baseVertex,
        int32 instanceCount,
        int32 baseInstance)
    {
        commandEncoder.DrawIndexed(
            primitiveType,
            (uint32)indexCount,
            indexType,
            indexBuffer,
            (uint32)indexBufferOffset,
            (uint32)instanceCount,
            (uint32)baseVertex,
            (uint32)baseInstance);
    }
}
