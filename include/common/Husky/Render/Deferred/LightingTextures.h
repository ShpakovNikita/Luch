#pragma once

#include <Husky/RefPtr.h>
#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/Render/RenderForwards.h>
#include <Husky/Render/Deferred/DeferredForwards.h>

namespace Husky::Render::Deferred
{
    using namespace Graphics;

    struct LightingTextures
    {
        RefPtr<Texture> diffuseLightingTexture;
        RefPtr<Texture> specularLightingTexture;
    };
}

