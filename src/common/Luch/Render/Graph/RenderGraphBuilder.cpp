#include <Luch/Render/Graph/RenderGraphBuilder.h>
#include <Luch/Graphics/GraphicsDevice.h>
#include <Luch/Graphics/CommandQueue.h>
#include <Luch/Graphics/CommandPool.h>
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
            return { false };
        }

        commandPool = std::move(createdPool);

        return true;
    }

    bool RenderGraphBuilder::Deinitialize()
    {
        device = nullptr;
        queue = nullptr;
        commandPool.Release();
        resourceManager.reset();
        return false;
    }

    RenderGraphNode* RenderGraphBuilder::AddRenderPass(const String& name, RenderGraphPass* pass)
    {
        RenderGraphNode* node = &(nodes.emplace_back());
        node->pass = pass;
        node->resourceManager = resourceManager.get();
        node->name = name;
        return node;
    }

    RenderGraphBuildResult RenderGraphBuilder::Build()
    {
        RenderGraphBuildResult result;
        resourceManager->Reset();

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
                    result.edges.emplace(i, j);
                    consumedResources.insert(intersection.begin(), intersection.end());
                }
            }

            std::set_difference(
                producedResources.begin(), producedResources.end(),
                consumedResources.begin(), consumedResources.end(),
                std::back_inserter(result.unusedResources));
        }

        auto order = TopologicalOrder(nodes.size(), result.edges);
        for(int32 index : order)
        {
            result.nodes.push_back(&nodes[index]);
        }

        result.resourcesCreated = resourceManager->Build();
        if(!result.resourcesCreated)
        {
            resourceManager->Reset();
        }

        return result;
    }

    RenderGraphExecuteResult RenderGraphBuilder::Execute(RenderGraphBuildResult& buildResult)
    {
        RenderGraphExecuteResult result;

        RefPtrVector<GraphicsCommandList> commandLists;
        int32 nodeCount = buildResult.nodes.size();

        commandLists.reserve(nodeCount);
        for(int32 i = 0; i < nodeCount; i++)
        {
            auto [createCommandListResult, createdCommandList] = commandPool->AllocateGraphicsCommandList();
            if(createCommandListResult != GraphicsResult::Success)
            {
                return result;
            }

            commandLists.emplace_back(std::move(createdCommandList));
        }

        result.commandLists = std::move(commandLists);

        for(int32 i = 0; i < nodeCount; i++)
        {
            nodes[i].pass->ExecuteGraphicsPass(result.commandLists[i]);
        }

        return result;
    }

    RenderGraphSubmitResult RenderGraphBuilder::Submit(RenderGraphExecuteResult& executeResult)
    {
        for(int32 i = 0; i < executeResult.commandLists.size(); i++)
        {
            auto result = queue->Submit(executeResult.commandLists[i]);
        }
    }
}