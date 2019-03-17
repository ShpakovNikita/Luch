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

        virtual void ExecuteGraphicsPass(
            [[maybe_unused]] RenderGraphResourceManager* manager,
            [[maybe_unused]] GraphicsCommandList* commandList) {};

        virtual void ExecuteComputePass(
            [[maybe_unused]] RenderGraphResourceManager* manager,
            [[maybe_unused]] ComputeCommandList* commandList) {};

        virtual void ExecuteCopyPass(
            [[maybe_unused]] RenderGraphResourceManager* manager,
            [[maybe_unused]] CopyCommandList* commandList) {};
    };

    inline RenderGraphPass::~RenderGraphPass() {}
}
