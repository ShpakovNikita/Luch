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
            GraphicsDevice* device,
            CommandQueue* queue,
            UniquePtr<RenderGraphResourceManager> resourceManager,
            RenderGraphData graphData);
    private:
        RenderGraphData graphData;
    };
}
