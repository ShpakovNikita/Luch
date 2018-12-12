#pragma once

#include <Luch/RefPtr.h>
#include <Luch/Render/Graph/RenderGraphForwards.h>
#include <Luch/Render/Graph/RenderGraphResources.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/RenderPassCreateInfo.h>

namespace Luch::Render::Graph
{
    using namespace Graphics;

    class RenderGraphResourceManager
    {
        struct AttachmentCreateInfo
        {
            int32 width = 0;
            int32 height = 0;
            Format format = Format::Undefined;
        };
    public:
        static constexpr int32 MaxColorAttachmentCount = RenderPassCreateInfo::MaxColorAttachmentCount;

        RenderGraphResourceManager(GraphicsDevice* device);

        RenderMutableResource ImportAttachment(RefPtr<Texture> texture);
        RenderMutableResource CreateAttachment(int32 width, int32 height, Format format);
        RenderMutableResource ModifyResource(RenderMutableResource resource);

        bool Build();

        RefPtr<Texture> GetTexture(RenderResource resource);
    private:
        GraphicsDevice* device = nullptr;
        RenderMutableResource GetNextHandle();
        uint32 nextHandleValue = 0;

        UnorderedMap<RenderResource, AttachmentCreateInfo> pendingAttachments;
        UnorderedMap<RenderResource, RenderMutableResource> modifiedResources;

        UnorderedMap<RenderResource, RefPtr<Texture>> createdTextures;
        UnorderedMap<RenderResource, RefPtr<Texture>> importedAttachments;
    };
}
