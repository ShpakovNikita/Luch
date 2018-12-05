#pragma once

#include <Luch/RefPtr.h>
#include <Luch/UniquePtr.h>
#include <Luch/Types.h>
#include <Luch/Graphics/BufferUsageFlags.h>
#include <Luch/Graphics/Format.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/TextureUsageFlags.h>
#include <Luch/Render/Graph/RenderGraphForwards.h>
#include <Luch/Render/Graph/RenderGraphResources.h>

namespace Luch::Render::Graph
{
    using namespace Graphics;

    enum struct RenderGraphBuildResult
    {
        Success,
        UnusedResources,
        //UnusedNodes,
        ResourceCreationFailed,
        CyclicDependency,
    };

    struct RenderGraphData
    {
        Vector<RenderGraphNode*> nodes;
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

        bool Execute();
        bool Submit();
    private:
        CommandQueue* queue = nullptr;
        RefPtr<CommandPool> commandPool;
        RefPtrVector<GraphicsCommandList> commandLists;
        UniquePtr<RenderGraphResourceManager> resourceManager;
        RenderGraphData graphData;
    };
}
