#include <Luch/Render/Graph/RenderGraph.h>
#include <Luch/Render/Graph/RenderGraphPass.h>
#include <Luch/Render/Graph/RenderGraphResourceManager.h>
#include <Luch/Graphics/CommandPool.h>
#include <Luch/Graphics/GraphicsCommandList.h>
#include <Luch/Graphics/ComputeCommandList.h>
#include <Luch/Graphics/CopyCommandList.h>

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

    RefPtrVector<CommandList> RenderGraph::Execute()
    {
        RefPtrVector<CommandList> commandLists;
        commandLists.reserve(graphData.nodes.size());
        for(const auto& node : graphData.nodes)
        {
            RefPtr<CommandList> commandList;

            switch(node.type)
            {
            case RenderGraphPassType::Compute:
            {
                auto [allocateResult, allocatedList] = commandPool->AllocateComputeCommandList();
                LUCH_ASSERT(allocateResult == GraphicsResult::Success);

                allocatedList->Begin();
                node.pass->ExecuteComputePass(
                    resourceManager.get(),
                    allocatedList);
                allocatedList->End();

                commandList = std::move(allocatedList);
                break;
            }
            case RenderGraphPassType::Graphics:
            {
                auto [allocateResult, allocatedList] = commandPool->AllocateGraphicsCommandList();
                LUCH_ASSERT(allocateResult == GraphicsResult::Success);

                allocatedList->Begin();
                allocatedList->BeginRenderPass(node.frameBuffer);
                node.pass->ExecuteGraphicsPass(
                    resourceManager.get(),
                    allocatedList);
                allocatedList->EndRenderPass();
                allocatedList->End();

                commandList = std::move(allocatedList);
                break;
            }
            case RenderGraphPassType::Copy:
            {
                auto [allocateResult, allocatedList] = commandPool->AllocateCopyCommandList();
                LUCH_ASSERT(allocateResult == GraphicsResult::Success);

                allocatedList->Begin();
                node.pass->ExecuteCopyPass(
                    resourceManager.get(),
                    allocatedList);
                allocatedList->End();

                commandList = std::move(allocatedList);
                break;
            }
            default:
                LUCH_ASSERT(false);
            }

            commandList->SetLabel(node.name);

            commandLists.push_back(commandList);
        }
        return commandLists;
    }
}
