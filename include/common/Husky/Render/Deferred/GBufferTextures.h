#pragma once

#include <Husky/RefPtr.h>
#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/Render/RenderForwards.h>
#include <Husky/Render/Deferred/DeferredForwards.h>

namespace Husky::Render::Deferred
{
    using namespace Graphics;

    struct GBufferTextures
    {
        RefPtr<Texture> baseColorTexture;
        RefPtr<Texture> normalMapTexture;
        RefPtr<Texture> depthStencilBuffer;
    };
}
