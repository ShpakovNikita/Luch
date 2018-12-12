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
        RenderGraphBuilder() = default;

        bool Initialize(GraphicsDevice* device, CommandQueue* queue);
        bool Deinitialize();

        UniquePtr<RenderGraphNodeBuilder> AddRenderPass(String name, RenderGraphPass* pass);
        ResultValue<RenderGraphBuildResult, UniquePtr<RenderGraph>> Build();
    private:
        GraphicsDevice* device = nullptr;
        CommandQueue* queue = nullptr;

        UniquePtr<RenderGraphResourceManager> resourceManager;
        RefPtr<CommandPool> commandPool;
        Vector<RenderGraphNode> renderGraphNodes;
    };
}
