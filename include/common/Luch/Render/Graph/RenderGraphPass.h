#pragma once

#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Render/Graph/RenderGraphForwards.h>

namespace Luch::Render::Graph
{
    using namespace Graphics;

    class RenderGraphPass
    {
    public:
        virtual ~RenderGraphPass() = 0;

        virtual void ExecuteGraphicsRenderPass(
            [[maybe_unused]] RenderGraphResourceManager* manager,
            [[maybe_unused]] FrameBuffer* frameBuffer,
            [[maybe_unused]] GraphicsCommandList* commandList) {};

        virtual void ExecuteComputeRenderPass(
            [[maybe_unused]] RenderGraphResourceManager* manager,
            [[maybe_unused]] ComputeCommandList* commandList) {};
    };

    inline RenderGraphPass::~RenderGraphPass() {}
}
