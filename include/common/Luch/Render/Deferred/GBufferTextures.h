#pragma once

#include <Luch/RefPtr.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Render/RenderForwards.h>
#include <Luch/Render/Deferred/DeferredForwards.h>

namespace Luch::Render::Deferred
{
    using namespace Graphics;

    struct GBufferTextures
    {
        RefPtr<Texture> baseColorTexture;
        RefPtr<Texture> normalMapTexture;
        RefPtr<Texture> depthStencilBuffer;
    };
}
