#pragma once

#include <Luch/RefPtr.h>
#include <Luch/Graphics/Format.h>
#include <Luch/Graphics/GraphicsForwards.h>

namespace Luch::Graphics
{
    struct TiledPipelineColorAttachmentState
    {
        Format format = Format::RGBA8Unorm;
    };

    struct TiledPipelineColorAttachmentsStateCreateInfo
    {
        Vector<TiledPipelineColorAttachmentState> attachments;
    };

    struct TiledPipelineStateCreateInfo
    {
        // We store shader programs in pipeline create info
        RefPtr<ShaderProgram> tiledProgram;

        TiledPipelineColorAttachmentsStateCreateInfo colorAttachments;
        PipelineLayout* pipelineLayout = nullptr;
        RenderPass* renderPass = nullptr;
        String name = "";
    };
}
