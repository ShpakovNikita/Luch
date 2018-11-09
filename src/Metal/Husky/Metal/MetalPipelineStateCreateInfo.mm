#include <Husky/Metal/MetalPipelineStateCreateInfo.h>
#include <Husky/Graphics/PipelineStateCreateInfo.h>
#include <Husky/Metal/MetalFormat.h>
#include <Husky/Metal/MetalShaderProgram.h>
#include <Husky/Metal/MetalPrimitiveTopology.h>
#include <Husky/Metal/MetalColorAttachment.h>
#include <Husky/Metal/MetalDepthStencilAttachment.h>
#include <Husky/Assert.h>

namespace Husky::Metal
{
    using namespace Graphics;

    mtlpp::VertexStepFunction ToMetalVertexStepFunction(VertexInputRate inputRate)
    {
        switch(inputRate)
        {
        case VertexInputRate::PerVertex:
            return mtlpp::VertexStepFunction::PerVertex;
        case VertexInputRate::PerInstance:
            return mtlpp::VertexStepFunction::PerInstance;
        default:
            HUSKY_ASSERT_MSG(false, "Unknown input rate");
            return mtlpp::VertexStepFunction::PerVertex;
        }
    }
    mtlpp::VertexDescriptor ToMetalVertexDescriptor(const PipelineVertexInputStateCreateInfo& ci)
    {
        mtlpp::VertexDescriptor d;

        auto mtlAttributes = d.GetAttributes();
        auto mtlLayouts = d.GetLayouts();

        // TODO
        int32 buffersStart = 31 - 1 - ci.bindings.size();

        for(int32 i = 0; i < ci.attributes.size(); i++)
        {
            const auto& attribute = ci.attributes[i];

            mtlAttributes[i].SetFormat(ToMetalVertexFormat(attribute.format));
            mtlAttributes[i].SetOffset((uint32)attribute.offset);
            mtlAttributes[i].SetBufferIndex((uint32)(buffersStart + attribute.binding)); // TODO
        }

        for(int32 i = 0; i < ci.bindings.size(); i++)
        {
            auto binding = ci.bindings[i];

            mtlLayouts[buffersStart + i].SetStride((uint32)binding.stride);
            mtlLayouts[buffersStart + i].SetStepRate(1);
            mtlLayouts[buffersStart + i].SetStepFunction(ToMetalVertexStepFunction(binding.inputRate));
        }

        return d;
    }

    mtlpp::RenderPipelineDescriptor ToMetalPipelineStateCreateInfo(const PipelineStateCreateInfo& ci)
    {
        mtlpp::RenderPipelineDescriptor d;
        auto mtlVertexProgram = static_cast<MetalShaderProgram*>(ci.vertexProgram.Get());
        auto mtlFragmentProgram = static_cast<MetalShaderProgram*>(ci.fragmentProgram.Get());
        d.SetVertexDescriptor(ToMetalVertexDescriptor(ci.inputAssembler));

        HUSKY_ASSERT(mtlVertexProgram != nullptr);
        d.SetVertexFunction(mtlVertexProgram->GetMetalFunction());

        if(mtlFragmentProgram != nullptr)
        {
            d.SetFragmentFunction(mtlFragmentProgram->GetMetalFunction());
        }
        
        d.SetInputPrimitiveTopology(ToMetalPrimitiveTopologyClass(ci.inputAssembler.primitiveTopology));
        // TODO multisampling
        d.SetRasterizationEnabled(!ci.rasterization.rasterizerDiscardEnable);

        for(int32 i = 0; i < ci.colorAttachments.attachments.size(); i++)
        {
            auto colorAttachment = ci.colorAttachments.attachments[i];
            d.GetColorAttachments()[i].SetPixelFormat(ToMetalPixelFormat(colorAttachment.format));
            d.GetColorAttachments()[i].SetBlendingEnabled(colorAttachment.blendEnable);
            d.GetColorAttachments()[i].SetSourceRgbBlendFactor(ToMetalBlendFactor(colorAttachment.srcColorBlendFactor));
            d.GetColorAttachments()[i].SetDestinationRgbBlendFactor(ToMetalBlendFactor(colorAttachment.dstColorBlendFactor));
            d.GetColorAttachments()[i].SetRgbBlendOperation(ToMetalBlendOperation(colorAttachment.colorBlendOp));
            d.GetColorAttachments()[i].SetSourceAlphaBlendFactor(ToMetalBlendFactor(colorAttachment.srcAlphaBlendFactor));
            d.GetColorAttachments()[i].SetDestinationAlphaBlendFactor(ToMetalBlendFactor(colorAttachment.dstAlphaBlendFactor));
            d.GetColorAttachments()[i].SetAlphaBlendOperation(ToMetalBlendOperation(colorAttachment.alphaBlendOp));
            d.GetColorAttachments()[i].SetWriteMask(ToMetalColorWriteMask(colorAttachment.colorWriteMask));
        }

        if(ci.depthStencil.depthTestEnable || ci.depthStencil.stencilTestEnable)
        {
            auto format = ci.depthStencil.depthStencilFormat;

            // TODO functions to check if format is depth or stencil only
            if(format != Format::S8Uint)
            {
                d.SetDepthAttachmentPixelFormat(ToMetalPixelFormat(ci.depthStencil.depthStencilFormat));
            }

            if(format != Format::D32Sfloat && format != Format::D16Unorm)
            {
                d.SetStencilAttachmentPixelFormat(ToMetalPixelFormat(ci.depthStencil.depthStencilFormat));
            }
        }

        return d;
    }

    mtlpp::StencilDescriptor ToMetalStencilDescriptor(const PipelineStencilStateCreateInfo& createInfo)
    {
        mtlpp::StencilDescriptor d;

        d.SetWriteMask(createInfo.writeMask);
        d.SetReadMask(createInfo.reference);
        d.SetDepthFailureOperation(ToMetalStencilOperation(createInfo.depthFailOperation));
        d.SetStencilFailureOperation(ToMetalStencilOperation(createInfo.stencilFailOperation));
        d.SetDepthStencilPassOperation(ToMetalStencilOperation(createInfo.depthStencilPassOperation));
        d.SetStencilCompareFunction(ToMetalCompareFunction(createInfo.compareFunction));

        return d;
    }

    mtlpp::DepthStencilDescriptor ToMetalDepthStencilDescriptor(const PipelineStateCreateInfo& createInfo)
    {
        mtlpp::DepthStencilDescriptor d;

        const auto& ci = createInfo.depthStencil;

        if(ci.depthWriteEnable)
        {
            d.SetDepthCompareFunction(ToMetalCompareFunction(ci.depthCompareFunction));
            d.SetDepthWriteEnabled(ci.depthWriteEnable);
        }

        if(ci.stencilTestEnable)
        {
            d.SetBackFaceStencil(ToMetalStencilDescriptor(ci.back));
            d.SetFrontFaceStencil(ToMetalStencilDescriptor(ci.front));
        }

        return d;
    }
}
