#include <Luch/Assert.h>
#include <Luch/Graphics/RenderPass.h>
#include <Luch/Graphics/RenderPassCreateInfo.h>
#include <Luch/Graphics/TextureCreateInfo.h>
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

        RenderMutableResource resource = resourceManager->ImportTexture(texture);
        node->colorAttachmentResources[index] = resource;

        node->importedResources.push_back(resource);

        return resource;
    }

    RenderMutableResource RenderGraphNodeBuilder::ImportDepthStencilAttachment(RefPtr<Texture> texture)
    {
        auto node = GetNode();
        const auto& attachment = node->renderPass->GetCreateInfo().depthStencilAttachment;

        LUCH_ASSERT(attachment.has_value());

        RenderMutableResource resource = resourceManager->ImportTexture(texture);
        node->depthStencilAttachmentResource = resource;

        node->importedResources.push_back(resource);

        return resource;
    }

    RenderMutableResource RenderGraphNodeBuilder::UseColorAttachment(int32 index, RenderMutableResource colorAttachmentHandle)
    {
        auto node = GetNode();
        const auto& attachment = node->renderPass->GetCreateInfo().colorAttachments[index];

        LUCH_ASSERT(attachment.has_value());

        RenderMutableResource resource = resourceManager->ModifyResource(colorAttachmentHandle);
        node->colorAttachmentResources[index] = resource;

        node->readResources.push_back(colorAttachmentHandle);
        node->writtenResources.push_back(resource);

        return resource;
    }

    RenderMutableResource RenderGraphNodeBuilder::UseDepthStencilAttachment(RenderMutableResource depthStencilAttachmentHandle)
    {
        auto node = GetNode();
        const auto& attachment = node->renderPass->GetCreateInfo().depthStencilAttachment;

        LUCH_ASSERT(attachment.has_value());

        RenderMutableResource resource = resourceManager->ModifyResource(depthStencilAttachmentHandle);
        node->depthStencilAttachmentResource = resource;

        node->readResources.push_back(depthStencilAttachmentHandle);
        node->writtenResources.push_back(resource);

        return resource;
    }

    RenderMutableResource RenderGraphNodeBuilder::CreateColorAttachment(int32 index, Size2i size, ResourceStorageMode storageMode)
    {
        auto node = GetNode();
        const auto& attachment = node->renderPass->GetCreateInfo().colorAttachments[index];

        LUCH_ASSERT(attachment.has_value());
        LUCH_ASSERT(attachment->format != Format::Undefined);

        TextureCreateInfo textureCreateInfo;
        textureCreateInfo.format = attachment->format;
        textureCreateInfo.width = size.width;
        textureCreateInfo.height = size.height;
        textureCreateInfo.usage = TextureUsageFlags::ColorAttachment | TextureUsageFlags::ShaderRead | TextureUsageFlags::ShaderWrite;
        textureCreateInfo.storageMode = storageMode;

        RenderMutableResource resource = resourceManager->CreateTexture(textureCreateInfo);
        node->colorAttachmentResources[index] = resource;

        node->createdResources.push_back(resource);

        return resource;
    }

    RenderMutableResource RenderGraphNodeBuilder::CreateDepthStencilAttachment(Size2i size, ResourceStorageMode storageMode)
    {
        auto node = GetNode();
        const auto& attachment = node->renderPass->GetCreateInfo().depthStencilAttachment;

        LUCH_ASSERT(attachment.has_value());
        LUCH_ASSERT(attachment->format != Format::Undefined);

        TextureCreateInfo textureCreateInfo;
        textureCreateInfo.format = attachment->format;
        textureCreateInfo.width = size.width;
        textureCreateInfo.height = size.height;
        textureCreateInfo.usage = TextureUsageFlags::DepthStencilAttachment | TextureUsageFlags::ShaderRead | TextureUsageFlags::ShaderWrite;
        textureCreateInfo.storageMode = storageMode;

        RenderMutableResource resource = resourceManager->CreateTexture(textureCreateInfo);
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

    RenderMutableResource RenderGraphNodeBuilder::ImportTexture(RefPtr<Texture> texture)
    {
        auto node = GetNode();
        auto resource = resourceManager->ImportTexture(texture);

        node->importedResources.push_back(resource);

        return resource;
    }

    RenderMutableResource RenderGraphNodeBuilder::CreateTexture(const TextureCreateInfo& createInfo)
    {
        auto node = GetNode();
        auto resource = resourceManager->CreateTexture(createInfo);

        node->createdResources.push_back(resource);

        return resource;
    }

    RenderMutableResource RenderGraphNodeBuilder::WritesToTexture(RenderMutableResource resource)
    {
        auto node = GetNode();
        auto modifiedResource = resourceManager->ModifyResource(resource);

        node->writtenResources.push_back(modifiedResource);

        return modifiedResource;
    }

    RenderResource RenderGraphNodeBuilder::ReadsTexture(RenderResource resource)
    {
        auto node = GetNode();
        node->readResources.push_back(resource);
        return resource;
    }

    RenderMutableResource RenderGraphNodeBuilder::ImportBuffer(RefPtr<Buffer> buffer)
    {
        auto node = GetNode();

        RenderMutableResource resource = resourceManager->ImportBuffer(buffer);
        node->importedResources.push_back(resource);

        return resource;
    }

    RenderMutableResource RenderGraphNodeBuilder::CreateBuffer(const BufferCreateInfo& createInfo)
    {
        auto node = GetNode();

        RenderMutableResource resource = resourceManager->CreateBuffer(createInfo);
        node->createdResources.push_back(resource);

        return resource;
    }

    RenderMutableResource RenderGraphNodeBuilder::WritesToBuffer(RenderMutableResource resource)
    {
        auto node = GetNode();

        RenderMutableResource modifiedResource = resourceManager->ModifyResource(resource);
        node->writtenResources.push_back(modifiedResource);

        return resource;
    }

    RenderResource RenderGraphNodeBuilder::ReadsBuffer(RenderResource resource)
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
