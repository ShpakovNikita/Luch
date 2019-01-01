#pragma once

#include <Luch/RefPtr.h>
#include <Luch/Render/Graph/RenderGraphForwards.h>
#include <Luch/Render/Graph/RenderGraphResources.h>
#include <Luch/Graphics/Size2.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/RenderPassCreateInfo.h>

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

        RenderMutableResource ImportAttachment(RefPtr<Texture> texture);
        RenderMutableResource ImportAttachmentDeferred();
        RenderMutableResource CreateAttachment(Size2i size, Format format);
        RenderMutableResource ModifyResource(RenderMutableResource resource);

        bool Build();

        void ProvideDeferredAttachment(RenderMutableResource handle, RefPtr<Texture> texture);
        RefPtr<Texture> GetTexture(RenderResource resource);
    private:
        GraphicsDevice* device = nullptr;
        RenderMutableResource GetNextHandle();
        uint32 nextHandleValue = 1;

        UnorderedMap<RenderResource, AttachmentCreateInfo> pendingAttachments;
        UnorderedMap<RenderResource, RenderMutableResource> modifiedResources;

        UnorderedMap<RenderResource, RefPtr<Texture>> acquiredTextures;
        UnorderedMap<RenderResource, RefPtr<Texture>> importedAttachments;

        RenderGraphResourcePool* pool = nullptr;
    };
}
