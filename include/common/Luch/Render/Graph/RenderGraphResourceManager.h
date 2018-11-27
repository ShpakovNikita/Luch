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
        RenderMutableResource CreateRenderTarget(const RenderTargetInfo& info);
        RenderMutableResource ImportRenderTarget(const RefPtr<Texture>& texture);

        RenderMutableResource ModifyResource(RenderMutableResource handle);

        RenderMutableResource CreateBuffer(BufferUsageFlags usageFlags);
        RenderMutableResource ImportBuffer(const RefPtr<Buffer>& buffer);
    private:
        RenderMutableResource GetNextHandle();
        uint32 nextHandleValue = 0;
        UnorderedMap<RenderMutableResource, RenderTargetInfo> pendingRenderTargets;
        UnorderedMap<RenderMutableResource, RefPtr<Texture>> importedRenderTargets;
        UnorderedMap<RenderMutableResource, BufferUsageFlags> pendingBuffers;
        UnorderedMap<RenderMutableResource, RefPtr<Buffer>> importedBuffers;
        UnorderedMap<RenderMutableResource, RenderMutableResource> modifiedResources;
    };
}
