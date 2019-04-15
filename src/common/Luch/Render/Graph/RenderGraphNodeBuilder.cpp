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

    void RenderGraphNodeBuilder::SetAttachmentSize(Size2i aAttachmentSize)
    {
        LUCH_ASSERT(!attachmentSize.has_value());
        attachmentSize = aAttachmentSize;
    }

    RenderMutableResource RenderGraphNodeBuilder::ImportColorAttachment(
        int32 index,
        RefPtr<Texture> texture,
        const RenderGraphAttachmentDescriptor& descriptor)
    {
        auto node = GetMutableNode();
        const auto& attachment = node->renderPass->GetCreateInfo().colorAttachments[index];

        LUCH_ASSERT(attachment.has_value());

        RenderMutableResource resource = resourceManager->ImportTexture(texture);
        auto& colorAttachment = node->colorAttachments[index];
        colorAttachment.resource = resource;
        colorAttachment.descriptor = descriptor;

        node->importedResources.insert(resource);

        return resource;
    }

    RenderMutableResource RenderGraphNodeBuilder::ImportDepthStencilAttachment(
        RefPtr<Texture> texture,
        const RenderGraphAttachmentDescriptor& descriptor)
    {
        auto node = GetMutableNode();
        const auto& attachment = node->renderPass->GetCreateInfo().depthStencilAttachment;

        LUCH_ASSERT(attachment.has_value());

        RenderMutableResource resource = resourceManager->ImportTexture(texture);
        node->depthStencilAttachment.resource = resource;
        node->depthStencilAttachment.descriptor = descriptor;

        node->importedResources.insert(resource);

        return resource;
    }

    RenderMutableResource RenderGraphNodeBuilder::UseColorAttachment(
        int32 index,
        RenderMutableResource colorAttachmentHandle,
        const RenderGraphAttachmentDescriptor& descriptor)
    {
        auto node = GetMutableNode();
        const auto& attachment = node->renderPass->GetCreateInfo().colorAttachments[index];

        LUCH_ASSERT(attachment.has_value());

        RenderMutableResource resource = resourceManager->ModifyResource(colorAttachmentHandle);
        auto& colorAttachment = node->colorAttachments[index];
        colorAttachment.resource = resource;
        colorAttachment.descriptor = descriptor;

        node->readResources.insert(colorAttachmentHandle);
        node->writtenResources.insert(resource);

        return resource;
    }

    RenderMutableResource RenderGraphNodeBuilder::UseDepthStencilAttachment(
        RenderMutableResource depthStencilAttachmentHandle,
        const RenderGraphAttachmentDescriptor& descriptor)
    {
        auto node = GetMutableNode();
        const auto& attachment = node->renderPass->GetCreateInfo().depthStencilAttachment;

        LUCH_ASSERT(attachment.has_value());

        RenderMutableResource resource = resourceManager->ModifyResource(depthStencilAttachmentHandle);
        node->depthStencilAttachment.resource = resource;
        node->depthStencilAttachment.descriptor = descriptor;

        node->readResources.insert(depthStencilAttachmentHandle);
        node->writtenResources.insert(resource);

        return resource;
    }

    RenderMutableResource RenderGraphNodeBuilder::CreateColorAttachment(
        int32 index,
        const RenderGraphAttachmentCreateInfo& createInfo,
        const RenderGraphAttachmentDescriptor& descriptor)
    {
        auto node = GetMutableNode();
        const auto& attachment = node->renderPass->GetCreateInfo().colorAttachments[index];

        LUCH_ASSERT(attachment.has_value());
        LUCH_ASSERT(attachment->format != Format::Undefined);

        LUCH_ASSERT(attachmentSize.has_value());

        TextureCreateInfo textureCreateInfo;
        textureCreateInfo.format = attachment->format;
        textureCreateInfo.textureType = createInfo.textureType;
        textureCreateInfo.mipmapLevelCount = createInfo.mipmapLevelCount;
        textureCreateInfo.width = attachmentSize->width;
        textureCreateInfo.height = attachmentSize->height;
        textureCreateInfo.storageMode = createInfo.storageMode;
        textureCreateInfo.usage = TextureUsageFlags::ColorAttachment | TextureUsageFlags::ShaderRead | TextureUsageFlags::ShaderWrite;

        RenderMutableResource resource = resourceManager->CreateTexture(textureCreateInfo);
        auto& colorAttachment = node->colorAttachments[index];
        colorAttachment.resource = resource;
        colorAttachment.descriptor = descriptor;

        node->createdResources.insert(resource);

        return resource;
    }

    RenderMutableResource RenderGraphNodeBuilder::CreateDepthStencilAttachment(
        const RenderGraphAttachmentCreateInfo& createInfo,
        const RenderGraphAttachmentDescriptor& descriptor)
    {
        auto node = GetMutableNode();
        const auto& attachment = node->renderPass->GetCreateInfo().depthStencilAttachment;

        LUCH_ASSERT(attachment.has_value());
        LUCH_ASSERT(attachment->format != Format::Undefined);

        TextureCreateInfo textureCreateInfo;
        textureCreateInfo.format = attachment->format;
        textureCreateInfo.textureType = createInfo.textureType;
        textureCreateInfo.mipmapLevelCount = createInfo.mipmapLevelCount;
        textureCreateInfo.width = attachmentSize->width;
        textureCreateInfo.height = attachmentSize->height;
        textureCreateInfo.storageMode = createInfo.storageMode;
        textureCreateInfo.usage = TextureUsageFlags::DepthStencilAttachment | TextureUsageFlags::ShaderRead | TextureUsageFlags::ShaderWrite;

        RenderMutableResource resource = resourceManager->CreateTexture(textureCreateInfo);
        node->depthStencilAttachment.resource = resource;
        node->depthStencilAttachment.descriptor = descriptor;

        node->createdResources.insert(resource);

        return resource;
    }

    RenderMutableResource RenderGraphNodeBuilder::WritesToColorAttachment(
        int32 index,
        RenderMutableResource resource,
        const RenderGraphAttachmentDescriptor& descriptor)
    {
        auto node = GetMutableNode();
        const auto& attachment = node->renderPass->GetCreateInfo().colorAttachments[index];

        LUCH_ASSERT(attachment.has_value());

        RenderMutableResource modifiedResource = resourceManager->ModifyResource(resource);
        auto& colorAttachment = node->colorAttachments[index];
        colorAttachment.resource = resource;
        colorAttachment.descriptor = descriptor;

        node->writtenResources.insert(modifiedResource);

        return modifiedResource;
    }

    RenderMutableResource RenderGraphNodeBuilder::WritesToDepthStencilAttachment(
        RenderMutableResource resource,
        const RenderGraphAttachmentDescriptor& descriptor)
    {
        auto node = GetMutableNode();
        const auto& attachment = node->renderPass->GetCreateInfo().depthStencilAttachment;

        LUCH_ASSERT(attachment.has_value());

        RenderMutableResource modifiedResource = resourceManager->ModifyResource(resource);
        node->depthStencilAttachment.resource = resource;
        node->depthStencilAttachment.descriptor = descriptor;

        node->writtenResources.insert(modifiedResource);

        return modifiedResource;
    }

    RenderMutableResource RenderGraphNodeBuilder::ImportTexture(RefPtr<Texture> texture)
    {
        auto node = GetMutableNode();
        auto resource = resourceManager->ImportTexture(texture);

        node->importedResources.insert(resource);

        return resource;
    }

    RenderMutableResource RenderGraphNodeBuilder::CreateTexture(const TextureCreateInfo& createInfo)
    {
        auto node = GetMutableNode();
        auto resource = resourceManager->CreateTexture(createInfo);

        node->createdResources.insert(resource);

        return resource;
    }

    RenderMutableResource RenderGraphNodeBuilder::WritesToTexture(RenderMutableResource resource)
    {
        auto node = GetMutableNode();
        auto modifiedResource = resourceManager->ModifyResource(resource);

        node->writtenResources.insert(modifiedResource);

        return modifiedResource;
    }

    RenderResource RenderGraphNodeBuilder::ReadsTexture(RenderResource resource)
    {
        auto node = GetMutableNode();
        node->readResources.insert(resource);
        return resource;
    }

    RenderMutableResource RenderGraphNodeBuilder::ImportBuffer(RefPtr<Buffer> buffer)
    {
        auto node = GetMutableNode();

        RenderMutableResource resource = resourceManager->ImportBuffer(buffer);
        node->importedResources.insert(resource);

        return resource;
    }

    RenderMutableResource RenderGraphNodeBuilder::CreateBuffer(const BufferCreateInfo& createInfo)
    {
        auto node = GetMutableNode();

        RenderMutableResource resource = resourceManager->CreateBuffer(createInfo);
        node->createdResources.insert(resource);

        return resource;
    }

    RenderMutableResource RenderGraphNodeBuilder::WritesToBuffer(RenderMutableResource resource)
    {
        auto node = GetMutableNode();

        RenderMutableResource modifiedResource = resourceManager->ModifyResource(resource);
        node->writtenResources.insert(modifiedResource);

        return resource;
    }

    RenderResource RenderGraphNodeBuilder::ReadsBuffer(RenderResource resource)
    {
        auto node = GetMutableNode();

        node->readResources.insert(resource);

        return resource;
    }

    const RenderGraphNode* RenderGraphNodeBuilder::GetNode() const
    {
        return &(graphBuilder->renderGraphNodes[nodeIndex]);
    }

    RenderGraphNode* RenderGraphNodeBuilder::GetMutableNode()
    {
        return &(graphBuilder->renderGraphNodes[nodeIndex]);
    }
}
