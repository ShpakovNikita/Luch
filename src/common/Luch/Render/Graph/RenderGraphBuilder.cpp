#include <Luch/Render/Graph/RenderGraphBuilder.h>
#include <Luch/Render/Graph/RenderGraphResourceManager.h>
#include <Luch/Render/Graph/RenderGraphNodeBuilder.h>
#include <Luch/Render/Graph/TopologicalSort.h>
#include <Luch/Graphics/GraphicsDevice.h>
#include <Luch/Graphics/RenderPassCreateInfo.h>
#include <Luch/Graphics/FrameBufferCreateInfo.h>
#include <Luch/Graphics/CommandPool.h>

namespace Luch::Render::Graph
{
    using namespace Graphics;

    RenderGraphBuilder::RenderGraphBuilder() = default;
    RenderGraphBuilder::~RenderGraphBuilder() = default;

    bool RenderGraphBuilder::Initialize(GraphicsDevice* aDevice, RefPtr<CommandPool> aCommandPool)
    {
        device = aDevice;
        commandPool = commandPool;
        resourceManager = MakeUnique<RenderGraphResourceManager>(device);

        return true;
    }

    bool RenderGraphBuilder::Deinitialize()
    {
        device = nullptr;
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

        for(auto& node : data.nodes)
        {
            RenderPassCreateInfo renderPassCreateInfo;
            renderPassCreateInfo.colorAttachments = node.colorAttachments;
            renderPassCreateInfo.depthStencilAttachment = node.depthStencilAttachment;
            renderPassCreateInfo.name = node.name;

            auto [createRenderPassResult, createdRenderPass] = device->CreateRenderPass(renderPassCreateInfo);
            if(createRenderPassResult != GraphicsResult::Success)
            {
                return { RenderGraphBuildResult::RenderPassCreationFailed };
            }

            node.renderPass = std::move(createdRenderPass);

            FrameBufferCreateInfo frameBufferCreateInfo;
            frameBufferCreateInfo.renderPass = node.renderPass;

            for(int32 i = 0; i < node.colorAttachmentResources.size(); i++)
            {
                RenderMutableResource colorAttachmentResource = node.colorAttachmentResources[i];
                if(colorAttachmentResource)
                {
                    const auto& colorTexture = resourceManager->GetTexture(colorAttachmentResource);
                    LUCH_ASSERT(colorTexture != nullptr);
                    frameBufferCreateInfo.colorTextures[i] = colorTexture;
                }
            }

            RenderMutableResource depthStencilAttachmentResource = node.depthStencilAttachmentResource;
            if(depthStencilAttachmentResource)
            {
                const auto& depthStencilTexture = resourceManager->GetTexture(depthStencilAttachmentResource);
                LUCH_ASSERT(depthStencilTexture != nullptr);
                frameBufferCreateInfo.depthStencilTexture = depthStencilTexture;
            }

            auto [createFrameBufferResult, createdFrameBuffer] = device->CreateFrameBuffer(frameBufferCreateInfo);
            if(createFrameBufferResult != GraphicsResult::Success)
            {
                return { RenderGraphBuildResult::RenderPassCreationFailed };
            }

            node.frameBuffer = std::move(createdFrameBuffer);
        }

        UniquePtr<RenderGraph> graph = MakeUnique<RenderGraph>(
            commandPool, std::move(resourceManager), std::move(data));

        return { result, std::move(graph) };
    }
}
