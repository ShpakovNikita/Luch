#include <Luch/Render/Graph/RenderGraph.h>
#include <Luch/Render/Graph/RenderGraphPass.h>
#include <Luch/Render/Graph/RenderGraphResourceManager.h>
#include <Luch/Graphics/CommandPool.h>

namespace Luch::Render::Graph
{
    RenderGraph::RenderGraph(
        CommandPool* aCommandPool,
        UniquePtr<RenderGraphResourceManager> aResourceManager,
        RenderGraphData aGraphData)
        : commandPool(aCommandPool)
        , resourceManager(std::move(aResourceManager))
        , graphData(std::move(aGraphData))
    {
    }

    RenderGraph::~RenderGraph() = default;

    RefPtrVector<GraphicsCommandList> RenderGraph::Execute()
    {
        RefPtrVector<GraphicsCommandList> commandLists;
        commandLists.reserve(graphData.nodes.size());
        for(const auto& node : graphData.nodes)
        {
            auto [allocateResult, allocatedList] = commandPool->AllocateGraphicsCommandList();
            LUCH_ASSERT(allocateResult == GraphicsResult::Success);
            
            node.pass->ExecuteRenderPass(
                resourceManager.get(),
                node.frameBuffer,
                allocatedList);

            commandLists.push_back(allocatedList);
        }
        return commandLists;
    }
}
