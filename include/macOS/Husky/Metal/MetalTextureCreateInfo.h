#pragma once

#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/Graphics/TextureType.h>
#include <Husky/Graphics/TextureUsageFlags.h>
#include <Husky/Metal/MetalForwards.h>

namespace Husky::Metal
{
    using namespace Graphics;

    mtlpp::TextureType ToMetalTextureType(TextureType textureType);
    mtlpp::TextureUsage ToMetalTextureUsage(TextureUsageFlags flags);
    mtlpp::TextureDescriptor ToMetalTextureDescriptor(const TextureCreateInfo& createInfo);
}
