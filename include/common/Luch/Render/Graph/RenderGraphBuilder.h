#pragma once

#include <Luch/RefPtr.h>
#include <Luch/Types.h>
#include <Luch/Graphics/BufferUsageFlags.h>
#include <Luch/Graphics/Format.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/TextureUsageFlags.h>
#include <Luch/Render/Graph/RenderGraphForwards.h>
#include <Luch/Render/Graph/RenderGraphResources.h>

namespace Luch::Render::Graph
{
    using namespace Graphics;

    class RenderGraphBuilder;

    class RenderGraphNode
    {
        friend class RenderGraphBuilder;
    public:
        RenderMutableResource CreateRenderTarget(const RenderTargetInfo& info);
        RenderMutableResource ImportRenderTarget(const RefPtr<Texture>& texture);

        RenderMutableResource CreateBuffer(BufferUsageFlags usageFlags);
        RenderMutableResource ImportBuffer(const RefPtr<Buffer>& buffer);

        void ReadsTexture(RenderResource textureResource);
        RenderMutableResource WritesToRenderTarget(
            RenderMutableResource renderTargetResource,
            TextureUsageFlags usageFlags);

        void ReadsBuffer(RenderResource bufferResource);
    private:
        RenderGraphPass* pass = nullptr;
        RenderGraphResourceManager* resourceManager = nullptr;
        Vector<RenderMutableResource> createdRenderTargets;
        Vector<RenderMutableResource> importedRenderTargets;
        Vector<RenderMutableResource> createdBuffers;
        Vector<RenderMutableResource> importedBuffers;
        Vector<RenderResource> readTextures;
        Vector<RenderMutableResource> writeTextures;
        Vector<RenderResource> readBuffers;
    };

    class RenderGraphBuilder
    {
    public:
        RenderGraphNode* AddRenderPass(RenderGraphPass* pass);
    private:
        RenderGraphResourceManager* resourceManager;
        Vector<RenderGraphNode> nodes;
        UnorderedMultimap<RenderGraphNode*, RenderGraphNode*> edges;
    };
}
