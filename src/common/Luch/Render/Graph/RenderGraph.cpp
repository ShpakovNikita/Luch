#include <Luch/Render/Graph/RenderGraph.h>
#include <Luch/Graphics/GraphicsDevice.h>
#include <Luch/Graphics/CommandQueue.h>
#include <Luch/Graphics/CommandPool.h>
#include <Luch/Render/Graph/RenderGraphPass.h>
#include <Luch/Render/Graph/RenderGraphBuilder.h>
#include <Luch/Render/Graph/RenderGraphResourceManager.h>

namespace Luch::Render::Graph
{
    using namespace Graphics;

    RenderGraph::RenderGraph(
        GraphicsDevice* aDevice,
        CommandQueue* aQueue,
        UniquePtr<RenderGraphResourceManager> aResourceManager,
        RenderGraphData aGraphData)
        : queue(aQueue)
        , resourceManager(std::move(aResourceManager))
        , graphData(std::move(aGraphData))
    {
    }

    bool RenderGraph::Execute()
    {
        int32 nodeCount = graphData.nodes.size();

        commandLists.reserve(nodeCount);
        for(int32 i = 0; i < nodeCount; i++)
        {
            auto [createCommandListResult, createdCommandList] = commandPool->AllocateGraphicsCommandList();
            if(createCommandListResult != GraphicsResult::Success)
            {
                return false;
            }

            commandLists.emplace_back(std::move(createdCommandList));
        }

        for(int32 i = 0; i < nodeCount; i++)
        {
            graphData.nodes[i]->pass->ExecuteGraphicsPass(commandLists[i]);
        }

        return true;
    }

    bool RenderGraph::Submit()
    {

        for(int32 i = 0; i < commandLists.size(); i++)
        {
            auto result = queue->Submit(commandLists[i]);
            LUCH_ASSERT(result == GraphicsResult::Success);
            if(result != GraphicsResult::Success)
            {
                return false;
            }
        }

        return true;
    }
}