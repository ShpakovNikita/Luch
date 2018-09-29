#pragma once

#include <Husky/Graphics/Format.h>
#include <Husky/Graphics/VertexInputRate.h>
#include <Husky/Graphics/PolygonMode.h>
#include <Husky/Graphics/CullMode.h>
#include <Husky/Graphics/FrontFace.h>
#include <Husky/Graphics/Color.h>
#include <Husky/Graphics/ColorComponentFlags.h>
#include <Husky/Graphics/BlendFactor.h>
#include <Husky/Graphics/BlendOperation.h>
#include <Husky/Graphics/PrimitiveTopology.h>
#include <Husky/Graphics/StencilOperation.h>
#include <Husky/Graphics/CompareFunction.h>
#include <Husky/Graphics/GraphicsForwards.h>

namespace Husky::Graphics
{
    struct VertexInputAttributeDescription
    {
        Format format = Format::Undefined;
        int32 offset = 0;
        int32 binding = 0;
    };

    struct VertexInputBindingDescription
    {
        int32 stride = 0;
        VertexInputRate inputRate = VertexInputRate::PerVertex;
    };

    struct PipelineVertexInputStateCreateInfo
    {
        Vector<VertexInputAttributeDescription> attributes;
        Vector<VertexInputBindingDescription> bindings;
        PrimitiveTopology primitiveTopology = PrimitiveTopology::TriangleList;
    };

    struct PipelineRasterizationStateCreateInfo
    {
        PolygonMode polygonMode = PolygonMode::Fill;
        CullMode cullMode = CullMode::None;
        FrontFace frontFace = FrontFace::Clockwise;
        float32 depthBiasConstantFactor = 0.0f;
        float32 depthBiasClamp = 0.0f;
        float32 depthBiasSlopeFactor = 0.0f;
        bool depthClampEnable = false;
        bool depthBiasEnable = false;
        bool rasterizerDiscardEnable = false;
    };

    struct PipelineMultisampleStateCreateInfo
    {
        int32 rasterizationSamples = 1;
        bool sampleShadingEnable = false;
        float32 minSampleShading = 0;
        bool alphaToCoverageEnable = false;
        bool alphaToOneEnable = false;
    };

    struct PipelineStencilStateCreateInfo
    {
        StencilOperation depthFailOperation = StencilOperation::Keep;
        StencilOperation stencilFailOperation = StencilOperation::Keep;
        StencilOperation depthStencilPassOperation = StencilOperation::Keep;
        CompareFunction compareFunction = CompareFunction::Never;
        uint32 compareMask = 0;
        uint32 writeMask = 0;
        uint32 reference = 0;
    };

    struct PipelineDepthStencilStateCreateInfo
    {
        bool depthTestEnable = false;
        bool depthWriteEnable = false;
        CompareFunction depthCompareFunction = CompareFunction::Always;
        bool depthBoundsTestEnable = false;
        bool stencilTestEnable = false;
        Format depthStencilFormat = Format::D32SfloatS8Uint;
        PipelineStencilStateCreateInfo front;
        PipelineStencilStateCreateInfo back;
    };

    struct PipelineColorAttachmentState
    {
        Format format = Format::R8G8B8A8Unorm;
        bool blendEnable = false;
        BlendFactor srcColorBlendFactor = BlendFactor::One;
        BlendFactor dstColorBlendFactor = BlendFactor::Zero;
        BlendOperation colorBlendOp = BlendOperation::Add;
        BlendFactor srcAlphaBlendFactor = BlendFactor::One;
        BlendFactor dstAlphaBlendFactor = BlendFactor::Zero;
        BlendOperation alphaBlendOp = BlendOperation::Add;
        ColorComponentFlags colorWriteMask = ColorComponentFlags::All;
    };

    struct PipelineColorAttachmentsStateCreateInfo
    {
        Vector<PipelineColorAttachmentState> attachments;
        ColorSNorm32 blendColor;
    };

    struct PipelineStateCreateInfo
    {
        PipelineVertexInputStateCreateInfo inputAssembler;
        ShaderProgram* vertexProgram = nullptr;
        ShaderProgram* fragmentProgram = nullptr;
        PipelineMultisampleStateCreateInfo multisampling;
        PipelineRasterizationStateCreateInfo rasterization;
        PipelineDepthStencilStateCreateInfo depthStencil;
        PipelineColorAttachmentsStateCreateInfo colorAttachments;
        PipelineLayout* pipelineLayout = nullptr;
        RenderPass* renderPass = nullptr;
    };
}
