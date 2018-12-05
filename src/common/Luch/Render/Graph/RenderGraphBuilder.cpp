#include <Luch/Render/Graph/RenderGraphBuilder.h>
#include <Luch/Graphics/GraphicsDevice.h>
#include <Luch/Graphics/CommandQueue.h>
#include <Luch/Graphics/CommandPool.h>
#include <Luch/Render/Graph/RenderGraph.h>
#include <Luch/Render/Graph/RenderGraphPass.h>
#include <Luch/Render/Graph/RenderGraphResourceManager.h>
#include <Luch/Render/Graph/TopologicalSort.h>

namespace Luch::Render::Graph
{
    RenderMutableResource RenderGraphNode::CreateRenderTarget(const RenderTargetInfo& info)
    {
        auto handle = resourceManager->CreateRenderTarget(info);
        createdRenderTargets.emplace_back(handle);
        return handle;
    }

    RenderMutableResource RenderGraphNode::ImportRenderTarget(const RefPtr<Texture>& texture)
    {
        auto handle = resourceManager->ImportRenderTarget(texture);
        importedRenderTargets.emplace_back(handle);
        return handle;
    }

    void RenderGraphNode::ReadsTexture(RenderResource textureResource)
    {
        readTextures.emplace_back(textureResource);
    }

    RenderMutableResource RenderGraphNode::WritesToRenderTarget(
        RenderMutableResource renderTargetResource,
        TextureUsageFlags usageFlags)
    {
        writeTextures.emplace_back(renderTargetResource);
        auto handle = resourceManager->ModifyResource(renderTargetResource);
        return handle;
    }

    RenderGraphBuilder::~RenderGraphBuilder() = default;

    bool RenderGraphBuilder::Initialize(GraphicsDevice* aDevice, CommandQueue* aQueue)
    {
        device = aDevice;
        queue = aQueue;

        resourceManager = MakeUnique<RenderGraphResourceManager>(device);

        auto [createPoolResult, createdPool] = queue->CreateCommandPool();
        if(createPoolResult != GraphicsResult::Success)
        {
            return false;
        }

        commandPool = std::move(createdPool);

        return true;
    }

    bool RenderGraphBuilder::Deinitialize()
    {
        device = nullptr;
        queue = nullptr;
        resourceManager.reset();
        return true;
    }

    RenderGraphNode* RenderGraphBuilder::AddRenderPass(const String& name, RenderGraphPass* pass)
    {
        RenderGraphNode* node = &(nodes.emplace_back());
        node->pass = pass;
        node->resourceManager = resourceManager.get();
        node->name = name;
        return node;
    }

    ResultValue<RenderGraphBuildResult, UniquePtr<RenderGraph>> RenderGraphBuilder::Build()
    {
        RenderGraphData data;

        for(int32 i = 0; i < nodes.size(); i++)
        {
            const auto& node = nodes[i];
            Set<RenderResource> producedResources;
            producedResources.insert(node.createdRenderTargets.begin(), node.createdRenderTargets.end());
            producedResources.insert(node.importedRenderTargets.begin(), node.importedRenderTargets.end());
            producedResources.insert(node.writeTextures.begin(), node.writeTextures.end());

            Set<RenderResource> consumedResources;

            for(int32 j = 0; j < nodes.size(); j++)
            {
                const auto& otherNode = nodes[j];
                Vector<RenderResource> intersection;

                std::set_intersection(
                    producedResources.begin(), producedResources.end(),
                    otherNode.readTextures.begin(), otherNode.readTextures.end(),
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
            resourceManager->Reset();
            return { RenderGraphBuildResult::ResourceCreationFailed };
        }

        auto [sortResult, order] = TopologicalOrder(nodes.size(), data.edges);

        if(sortResult != TopologicalSortResult::Success)
        {
            return { RenderGraphBuildResult::CyclicDependency };
        }

        auto result = data.unusedResources.empty() 
            ? RenderGraphBuildResult::Success
            : RenderGraphBuildResult::UnusedResources;

        for(int32 index : order)
        {
            data.nodes.push_back(&nodes[index]);
        }

        UniquePtr<RenderGraph> graph = MakeUnique<RenderGraph>(
            device, queue, std::move(resourceManager), std::move(data));

        return { result, std::move(graph) };
    }
}