#pragma once

#include <Luch/RefPtr.h>
#include <Luch/Graphics/Format.h>
#include <Luch/Graphics/VertexInputRate.h>
#include <Luch/Graphics/PolygonMode.h>
#include <Luch/Graphics/CullMode.h>
#include <Luch/Graphics/FrontFace.h>
#include <Luch/Graphics/Color.h>
#include <Luch/Graphics/ColorComponentFlags.h>
#include <Luch/Graphics/BlendFactor.h>
#include <Luch/Graphics/BlendOperation.h>
#include <Luch/Graphics/PrimitiveTopology.h>
#include <Luch/Graphics/StencilOperation.h>
#include <Luch/Graphics/CompareFunction.h>
#include <Luch/Graphics/GraphicsForwards.h>

namespace Luch::Graphics
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

    struct GraphicsPipelineVertexInputStateCreateInfo
    {
        Vector<VertexInputAttributeDescription> attributes;
        Vector<VertexInputBindingDescription> bindings;
        PrimitiveTopology primitiveTopology = PrimitiveTopology::TriangleList;
    };

    struct GraphicsPipelineRasterizationStateCreateInfo
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

    struct GraphicsPipelineMultisampleStateCreateInfo
    {
        int32 rasterizationSamples = 1;
        bool sampleShadingEnable = false;
        float32 minSampleShading = 0;
        bool alphaToCoverageEnable = false;
        bool alphaToOneEnable = false;
    };

    struct GraphicsPipelineStencilStateCreateInfo
    {
        StencilOperation depthFailOperation = StencilOperation::Keep;
        StencilOperation stencilFailOperation = StencilOperation::Keep;
        StencilOperation depthStencilPassOperation = StencilOperation::Keep;
        CompareFunction compareFunction = CompareFunction::Never;
        uint32 readMask = 0xfffffff;
        uint32 writeMask = 0xffffffff;
        uint32 reference = 0;
    };

    struct GraphicsPipelineDepthStencilStateCreateInfo
    {
        bool depthTestEnable = false;
        bool stencilTestEnable = false;
        bool depthWriteEnable = false;
        CompareFunction depthCompareFunction = CompareFunction::Always;
        bool depthBoundsTestEnable = false;
        Format depthStencilFormat = Format::D32SfloatS8Uint;
        GraphicsPipelineStencilStateCreateInfo front;
        GraphicsPipelineStencilStateCreateInfo back;
    };

    struct GraphicsPipelineColorAttachmentState
    {
        Format format = Format::RGBA8Unorm;
        bool blendEnable = false;
        BlendFactor srcColorBlendFactor = BlendFactor::One;
        BlendFactor dstColorBlendFactor = BlendFactor::Zero;
        BlendOperation colorBlendOp = BlendOperation::Add;
        BlendFactor srcAlphaBlendFactor = BlendFactor::One;
        BlendFactor dstAlphaBlendFactor = BlendFactor::Zero;
        BlendOperation alphaBlendOp = BlendOperation::Add;
        ColorComponentFlags colorWriteMask = ColorComponentFlags::All;
    };

    struct GraphicsPipelineColorAttachmentsStateCreateInfo
    {
        Vector<GraphicsPipelineColorAttachmentState> attachments;
        ColorSNorm32 blendColor;
    };

    struct GraphicsPipelineStateCreateInfo
    {
        // We store shader programs in pipeline create info
        RefPtr<ShaderProgram> vertexProgram;
        RefPtr<ShaderProgram> fragmentProgram;

        GraphicsPipelineVertexInputStateCreateInfo inputAssembler;
        GraphicsPipelineMultisampleStateCreateInfo multisampling;
        GraphicsPipelineRasterizationStateCreateInfo rasterization;
        GraphicsPipelineDepthStencilStateCreateInfo depthStencil;
        GraphicsPipelineColorAttachmentsStateCreateInfo colorAttachments;
        PipelineLayout* pipelineLayout = nullptr;
        RenderPass* renderPass = nullptr;
        String name = "";
    };
}
