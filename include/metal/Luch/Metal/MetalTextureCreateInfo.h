#pragma once

#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/TextureType.h>
#include <Luch/Graphics/TextureUsageFlags.h>
#include <Luch/Metal/MetalForwards.h>

namespace Luch::Metal
{
    using namespace Graphics;

    mtlpp::TextureType ToMetalTextureType(TextureType textureType);
    mtlpp::TextureUsage ToMetalTextureUsage(TextureUsageFlags flags);
    mtlpp::TextureDescriptor ToMetalTextureDescriptor(const TextureCreateInfo& createInfo);
}
