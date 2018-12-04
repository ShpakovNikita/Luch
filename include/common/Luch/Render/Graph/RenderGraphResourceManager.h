#pragma once

#include <Luch/RefPtr.h>
#include <Luch/Types.h>
#include <Luch/Graphics/BufferUsageFlags.h>
#include <Luch/Graphics/Format.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/TextureUsageFlags.h>
#include <Luch/Render/Graph/RenderGraphResources.h>

namespace Luch::Render::Graph
{
    using namespace Graphics;

    class RenderGraphResourceManager
    {
    public:
        RenderGraphResourceManager(GraphicsDevice* device);

        RenderMutableResource CreateRenderTarget(const RenderTargetInfo& info);
        RenderMutableResource ImportRenderTarget(const RefPtr<Texture>& texture);

        RenderMutableResource ModifyResource(RenderMutableResource handle);

        void MarkUnused(const Vector<RenderResource>& unusedResources);

        bool Build();
        void Reset();

        RefPtr<Texture> GetTexture(RenderResource textureHandle);
    private:
        GraphicsDevice* device = nullptr;
        RenderMutableResource GetNextHandle();
        uint32 nextHandleValue = 0;

        UnorderedMap<RenderMutableResource, RenderTargetInfo> pendingRenderTargets;
        UnorderedMap<RenderResource, RenderMutableResource> modifiedResources;

        UnorderedMap<RenderResource, RefPtr<Texture>> createdTextures;
        UnorderedMap<RenderResource, RefPtr<Texture>> importedRenderTargets;
    };
}
