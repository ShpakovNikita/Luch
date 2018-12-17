#pragma once

#include <Luch/UniquePtr.h>
#include <Luch/ResultValue.h>
#include <Luch/Render/Graph/RenderGraph.h>
#include <Luch/Render/Graph/RenderGraphNode.h>
#include <Luch/Render/Graph/RenderGraphForwards.h>

namespace Luch::Render::Graph
{
    class RenderGraphBuilder
    {
        friend class RenderGraphNodeBuilder;
    public:
        RenderGraphBuilder();
        ~RenderGraphBuilder();

        bool Initialize(GraphicsDevice* device, RefPtr<CommandPool> commandPool);
        bool Deinitialize();

        UniquePtr<RenderGraphNodeBuilder> AddRenderPass(String name, RenderGraphPass* pass);
        ResultValue<RenderGraphBuildResult, UniquePtr<RenderGraph>> Build();
    private:
        GraphicsDevice* device = nullptr;
        RefPtr<CommandPool> commandPool;

        UniquePtr<RenderGraphResourceManager> resourceManager;
        Vector<RenderGraphNode> renderGraphNodes;
    };
}
