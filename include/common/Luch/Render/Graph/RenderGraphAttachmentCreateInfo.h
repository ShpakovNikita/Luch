#pragma once

#include <Luch/RefPtr.h>
#include <Luch/Size2.h>
#include <Luch/Render/Graph/RenderGraphForwards.h>
#include <Luch/Render/Graph/RenderGraphAttachment.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/TextureType.h>
#include <Luch/Graphics/ResourceStorageMode.h>

namespace Luch::Render::Graph
{
    using namespace Graphics;

    struct RenderGraphAttachmentCreateInfo
    {
        ResourceStorageMode storageMode = ResourceStorageMode::DeviceLocal;
        TextureType textureType = TextureType::Texture2D;
        int32 mipmapLevelCount = 1;
    };
}
