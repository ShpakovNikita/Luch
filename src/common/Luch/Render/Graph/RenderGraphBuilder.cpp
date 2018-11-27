#include <Luch/Render/Graph/RenderGraphBuilder.h>
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

    RenderMutableResource RenderGraphNode::CreateBuffer(BufferUsageFlags usageFlags)
    {
        auto handle = resourceManager->CreateBuffer(usageFlags);
        createdBuffers.emplace_back(handle);
        return handle;
    }

    RenderMutableResource RenderGraphNode::ImportBuffer(const RefPtr<Buffer>& buffer)
    {
        auto handle = resourceManager->ImportBuffer(buffer);
        importedBuffers.emplace_back(handle);
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

    void RenderGraphNode::ReadsBuffer(RenderResource bufferResource)
    {
        readBuffers.emplace_back(bufferResource);
    }

    RenderGraphNode* RenderGraphBuilder::AddRenderPass(RenderGraphPass* pass)
    {
        RenderGraphNode* node = &(nodes.emplace_back());
        node->pass = pass;
        node->resourceManager = resourceManager;
        return node;
    }
}