#pragma once

#include <Luch/UniquePtr.h>
#include <Luch/Render/Graph/RenderGraphNode.h>
#include <Luch/Render/Graph/RenderGraphForwards.h>

namespace Luch::Render::Graph
{
    enum struct RenderGraphBuildResult
    {
        Success,
        UnusedResources,
        ResourceCreationFailed,
        RenderPassCreationFailed,
        CyclicDependency,
    };

    struct RenderGraphData
    {
        Vector<RenderGraphNode> nodes;
        UnorderedMultimap<int32, int32> edges;
        Vector<RenderResource> unusedResources;
    };

    class RenderGraph
    {
    public:
        RenderGraph(
            CommandPool* commandPool,
            UniquePtr<RenderGraphResourceManager> resourceManager,
            RenderGraphData graphData);
        ~RenderGraph();

        RenderGraphResourceManager* GetResourceManager() { return resourceManager.get(); }

        RefPtrVector<CommandList> Execute();
    private:
        CommandPool* commandPool = nullptr;
        UniquePtr<RenderGraphResourceManager> resourceManager;
        RenderGraphData graphData;
    };
}
