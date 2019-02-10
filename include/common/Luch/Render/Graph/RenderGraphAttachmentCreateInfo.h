#pragma once

#include <Luch/RefPtr.h>
#include <Luch/Render/Graph/RenderGraphForwards.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/TextureType.h>
#include <Luch/Graphics/ResourceStorageMode.h>
#include <Luch/Graphics/Size2.h>

namespace Luch::Render::Graph
{
    using namespace Graphics;

    struct RenderGraphAttachmentCreateInfo
    {
        Size2i size;
        TextureType textureType;
        ResourceStorageMode storageMode;
    };
}
