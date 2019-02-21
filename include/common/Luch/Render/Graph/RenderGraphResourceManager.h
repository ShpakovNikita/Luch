#pragma once

#include <Luch/RefPtr.h>
#include <Luch/Render/Graph/RenderGraphForwards.h>
#include <Luch/Render/Graph/RenderGraphResources.h>
#include <Luch/Graphics/Size2.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/RenderPassCreateInfo.h>
#include <Luch/Graphics/BufferCreateInfo.h>

namespace Luch::Render::Graph
{
    using namespace Graphics;

    class RenderGraphResourceManager
    {
        struct AttachmentCreateInfo
        {
            Size2i size;
            Format format = Format::Undefined;
        };
    public:
        static constexpr int32 MaxColorAttachmentCount = RenderPassCreateInfo::MaxColorAttachmentCount;

        RenderGraphResourceManager(GraphicsDevice* device, RenderGraphResourcePool* pool);
        ~RenderGraphResourceManager();

        RenderMutableResource ImportTexture(RefPtr<Texture> texture);
        RenderMutableResource ImportTextureDeferred();
        RenderMutableResource CreateTexture(const TextureCreateInfo& createInfo);

        RenderMutableResource ImportBuffer(RefPtr<Buffer> buffer);
        RenderMutableResource CreateBuffer(const BufferCreateInfo& createInfo);

        RenderMutableResource ModifyResource(RenderMutableResource resource);

        bool Build();

        void ProvideDeferredTexture(RenderMutableResource handle, RefPtr<Texture> texture);

        RefPtr<Texture> GetTexture(RenderResource resource);
        RefPtr<Buffer> GetBuffer(RenderResource resource);

        RefPtr<Texture> ReleaseTexture(RenderResource resource);
        RefPtr<Buffer> ReleaseBuffer(RenderResource resource);
    private:
        GraphicsDevice* device = nullptr;
        RenderMutableResource GetNextHandle();
        uint32 nextHandleValue = 1;

        UnorderedMap<RenderResource, TextureCreateInfo> pendingTextures;
        UnorderedMap<RenderResource, BufferCreateInfo> pendingBuffers;

        UnorderedMap<RenderResource, RenderMutableResource> modifiedResources;

        UnorderedMap<RenderResource, RefPtr<Texture>> acquiredTextures;
        UnorderedMap<RenderResource, RefPtr<Texture>> importedTextures;

        UnorderedMap<RenderResource, RefPtr<Buffer>> acquiredBuffers;
        UnorderedMap<RenderResource, RefPtr<Buffer>> importedBuffers;

        RenderGraphResourcePool* pool = nullptr;
    };
}
