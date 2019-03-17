#pragma once

#include <Luch/RefPtr.h>
#include <Luch/Size2.h>
#include <Luch/Render/Graph/RenderGraphForwards.h>
#include <Luch/Render/Graph/RenderGraphResources.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/TextureType.h>
#include <Luch/Graphics/ResourceStorageMode.h>

namespace Luch::Render::Graph
{
    using namespace Graphics;

    struct RenderGraphAttachmentDescriptor
    {
        int32 mipmapLevel = 0;
        int32 slice = 0;
        int32 depthPlane = 0;
    };

    struct RenderGraphAttachment
    {
        RenderMutableResource resource;
        RenderGraphAttachmentDescriptor descriptor;
    };
}
