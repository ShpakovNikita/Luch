#include <Luch/Render/Graph/RenderGraphBuilder.h>
#include <Luch/Render/Graph/RenderGraphResourceManager.h>
#include <Luch/Render/Graph/RenderGraphNodeBuilder.h>
#include <Luch/Render/Graph/TopologicalSort.h>
#include <Luch/Graphics/CommandQueue.h>
#include <Luch/Graphics/CommandPool.h>

namespace Luch::Render::Graph
{
    using namespace Graphics;

    bool RenderGraphBuilder::Initialize(GraphicsDevice* aDevice, CommandQueue* aQueue)
    {
        device = aDevice;
        queue = aQueue;

        auto [createCommandPoolResult, createdCommandPool] = queue->CreateCommandPool();
        if(createCommandPoolResult != GraphicsResult::Success)
        {
            return false;
        }

        commandPool = std::move(createdCommandPool);

        resourceManager = MakeUnique<RenderGraphResourceManager>(device);

        return true;
    }

    bool RenderGraphBuilder::Deinitialize()
    {
        device = nullptr;
        queue = nullptr;
        commandPool.Release();
        resourceManager.reset();
        return true;
    }

    UniquePtr<RenderGraphNodeBuilder> RenderGraphBuilder::AddRenderPass(String name, RenderGraphPass* pass)
    {
        auto& node = renderGraphNodes.emplace_back();
        node.name = name;
        node.pass = pass;

        return MakeUnique<RenderGraphNodeBuilder>(this, renderGraphNodes.size() - 1, resourceManager.get());
    }

    ResultValue<RenderGraphBuildResult, UniquePtr<RenderGraph>> RenderGraphBuilder::Build()
    {
        RenderGraphData data;

        for(int32 i = 0; i < renderGraphNodes.size(); i++)
        {
            const auto& node = renderGraphNodes[i];
            Set<RenderResource> producedResources;
            producedResources.insert(node.createdResources.begin(), node.createdResources.end());
            producedResources.insert(node.importedResources.begin(), node.importedResources.end());
            producedResources.insert(node.writtenResources.begin(), node.writtenResources.end());

            Set<RenderResource> consumedResources;

            for(int32 j = 0; j < renderGraphNodes.size(); j++)
            {
                const auto& otherNode = renderGraphNodes[j];
                Vector<RenderResource> intersection;

                std::set_intersection(
                    producedResources.begin(), producedResources.end(),
                    otherNode.readResources.begin(), otherNode.readResources.end(),
                    std::back_inserter(intersection));

                if(!intersection.empty())
                {
                    LUCH_ASSERT(i != j);
                    data.edges.emplace(i, j);
                    consumedResources.insert(intersection.begin(), intersection.end());
                }
            }

            std::set_difference(
                producedResources.begin(), producedResources.end(),
                consumedResources.begin(), consumedResources.end(),
                std::back_inserter(data.unusedResources));
        }

        bool resourcesCreated = resourceManager->Build();
        if(!resourcesCreated)
        {
            return { RenderGraphBuildResult::ResourceCreationFailed };
        }

        auto [sortResult, order] = TopologicalOrder(renderGraphNodes.size(), data.edges);

        if(sortResult != TopologicalSortResult::Success)
        {
            return { RenderGraphBuildResult::CyclicDependency };
        }

        auto result = data.unusedResources.empty() 
            ? RenderGraphBuildResult::Success
            : RenderGraphBuildResult::UnusedResources;

        for(int32 index : order)
        {
            data.nodes.emplace_back(std::move(renderGraphNodes[index]));
        }

        UniquePtr<RenderGraph> graph = MakeUnique<RenderGraph>(
            device, queue, std::move(resourceManager), std::move(data));

        return { result, std::move(graph) };
    }
}
