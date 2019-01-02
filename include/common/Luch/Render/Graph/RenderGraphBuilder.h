#pragma once

#include <Luch/RefPtr.h>
#include <Luch/UniquePtr.h>
#include <Luch/ResultValue.h>
#include <Luch/Graphics/GraphicsForwards.h>
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

        bool Initialize(
            GraphicsDevice* device,
            RefPtr<CommandPool> commandPool,
            RenderGraphResourcePool* resourcePool);

        bool Deinitialize();

        UniquePtr<RenderGraphNodeBuilder> AddRenderPass(
            String name,
            RefPtr<Graphics::RenderPass> renderPass,
            RenderGraphPass* pass);

        ResultValue<RenderGraphBuildResult, UniquePtr<RenderGraph>> Build();

        RenderGraphResourceManager* GetResourceManager() { return resourceManager.get(); }
    private:
        GraphicsDevice* device = nullptr;
        RefPtr<CommandPool> commandPool;

        UniquePtr<RenderGraphResourceManager> resourceManager;
        Vector<RenderGraphNode> renderGraphNodes;
    };
}
