#include <Luch/Assert.h>
#include <Luch/Graphics/RenderPass.h>
#include <Luch/Graphics/RenderPassCreateInfo.h>
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
        RefPtr<Texture> texture)
    {
        auto node = GetNode();
        const auto& attachment = node->renderPass->GetCreateInfo().colorAttachments[index];

        LUCH_ASSERT(attachment.has_value());

        RenderMutableResource resource = resourceManager->ImportAttachment(texture);
        node->colorAttachmentResources[index] = resource;

        node->importedResources.push_back(resource);

        return resource;
    }

    RenderMutableResource RenderGraphNodeBuilder::ImportDepthStencilAttachment(RefPtr<Texture> texture)
    {
        auto node = GetNode();
        const auto& attachment = node->renderPass->GetCreateInfo().depthStencilAttachment;

        LUCH_ASSERT(attachment.has_value());

        RenderMutableResource resource = resourceManager->ImportAttachment(texture);
        node->depthStencilAttachmentResource = resource;

        node->importedResources.push_back(resource);

        return resource;
    }

    RenderMutableResource RenderGraphNodeBuilder::CreateColorAttachment(int32 index, Size2i size)
    {
        auto node = GetNode();
        const auto& attachment = node->renderPass->GetCreateInfo().colorAttachments[index];

        LUCH_ASSERT(attachment.has_value());

        RenderMutableResource resource = resourceManager->CreateAttachment(size, attachment->format);
        node->colorAttachmentResources[index] = resource;

        node->createdResources.push_back(resource);

        return resource;
    }

    RenderMutableResource RenderGraphNodeBuilder::CreateDepthStencilAttachment(Size2i size)
    {
        auto node = GetNode();
        const auto& attachment = node->renderPass->GetCreateInfo().depthStencilAttachment;

        LUCH_ASSERT(attachment.has_value());

        RenderMutableResource resource = resourceManager->CreateAttachment(size, attachment->format);
        node->depthStencilAttachmentResource = resource;

        node->createdResources.push_back(resource);

        return resource;
    }

    RenderMutableResource RenderGraphNodeBuilder::WritesToColorAttachment(int32 index, RenderMutableResource resource)
    {
        auto node = GetNode();
        const auto& attachment = node->renderPass->GetCreateInfo().colorAttachments[index];
        
        LUCH_ASSERT(attachment.has_value());

        RenderMutableResource modifiedResource = resourceManager->ModifyResource(resource);
        node->colorAttachmentResources[index] = resource;

        node->writtenResources.push_back(modifiedResource);

        return modifiedResource;
    }

    RenderMutableResource RenderGraphNodeBuilder::WritesToDepthStencilAttachment(RenderMutableResource resource)
    {
        auto node = GetNode();
        const auto& attachment = node->renderPass->GetCreateInfo().depthStencilAttachment;

        LUCH_ASSERT(attachment.has_value());

        RenderMutableResource modifiedResource = resourceManager->ModifyResource(resource);
        node->depthStencilAttachmentResource = resource;

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
