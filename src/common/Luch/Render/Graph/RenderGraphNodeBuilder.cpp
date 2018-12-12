#include <Luch/Assert.h>
#include <Luch/Render/Graph/RenderGraphResources.h>
#include <Luch/Render/Graph/RenderGraphResourceManager.h>
#include <Luch/Render/Graph/RenderGraphNode.h>
#include <Luch/Render/Graph/RenderGraphBuilder.h>
#include <Luch/Render/Graph/RenderGraphNodeBuilder.h>

namespace Luch::Render::Graph
{
    RenderGraphNodeBuilder::RenderGraphNodeBuilder(
        RenderGraphBuilder* aGraphBuilder,
        int32 aNodeIndex,
        RenderGraphResourceManager* aResourceManager)
        : graphBuilder(aGraphBuilder)
        , nodeIndex(aNodeIndex)
        , resourceManager(aResourceManager)
    {
    }

    RenderMutableResource RenderGraphNodeBuilder::ImportColorAttachment(
        int32 index,
        RefPtr<Texture> texture,
        const ColorAttachment& attachment)
    {
        auto node = GetNode();
        LUCH_ASSERT(!node->colorAttachments[index].has_value());

        node->colorAttachments[index] = attachment;
        RenderMutableResource resource = resourceManager->ImportAttachment(texture);

        node->importedResources.push_back(resource);

        return resource;
    }

    RenderMutableResource RenderGraphNodeBuilder::ImportDepthStencilAttachment(RefPtr<Texture> texture, const DepthStencilAttachment& attachment)
    {
        auto node = GetNode();

        LUCH_ASSERT(!node->depthStencilAttachment.has_value());

        node->depthStencilAttachment = attachment;
        RenderMutableResource resource = resourceManager->ImportAttachment(texture);

        node->importedResources.push_back(resource);

        return resource;
    }

    RenderMutableResource RenderGraphNodeBuilder::CreateColorAttachment(int32 index, int32 width, int32 height, const ColorAttachment& attachment)
    {
        auto node = GetNode();

        LUCH_ASSERT(!node->colorAttachments[index].has_value());

        node->colorAttachments[index] = attachment;
        RenderMutableResource resource = resourceManager->CreateAttachment(width, height, attachment.format);

        node->createdResources.push_back(resource);

        return resource;
    }

    RenderMutableResource RenderGraphNodeBuilder::CreateDepthStencilAttachment(int32 width, int32 height, const DepthStencilAttachment& attachment)
    {
        auto node = GetNode();

        LUCH_ASSERT(!node->depthStencilAttachment.has_value());

        node->depthStencilAttachment = attachment;
        RenderMutableResource resource = resourceManager->CreateAttachment(width, height, attachment.format);

        node->createdResources.push_back(resource);

        return resource;
    }

    RenderMutableResource RenderGraphNodeBuilder::WritesToColorAttachment(int32 index, RenderMutableResource resource, const ColorAttachment& attachment)
    {
        auto node = GetNode();

        LUCH_ASSERT(!node->colorAttachments[index].has_value());

        node->colorAttachments[index] = attachment;
        RenderMutableResource modifiedResource = resourceManager->ModifyResource(resource);

        node->writtenResources.push_back(modifiedResource);

        return modifiedResource;
    }

    RenderMutableResource RenderGraphNodeBuilder::WritesToDepthStencilAttachment(RenderMutableResource resource, const DepthStencilAttachment& attachment)
    {
        auto node = GetNode();

        LUCH_ASSERT(!node->depthStencilAttachment.has_value());

        node->depthStencilAttachment = attachment;
        RenderMutableResource modifiedResource = resourceManager->ModifyResource(resource);

        node->writtenResources.push_back(modifiedResource);

        return modifiedResource;
    }

    RenderResource RenderGraphNodeBuilder::ReadsTexture(RenderResource resource)
    {
        auto node = GetNode();
        node->readResources.push_back(resource);
        return resource;
    }

    RenderGraphNode* RenderGraphNodeBuilder::GetNode() const
    {
        return &(graphBuilder->renderGraphNodes[nodeIndex]);
    }
}
