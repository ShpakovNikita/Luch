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

        virtual void ExecuteRenderPass(
            RenderGraphResourceManager* manager,
            FrameBuffer* frameBuffer, 
            GraphicsCommandList* commandList) = 0;
    };

    inline RenderGraphPass::~RenderGraphPass() {}
}
