#pragma once

#include <Husky/Types.h>
#include <Husky/Graphics/TextureType.h>
#include <Husky/Graphics/TextureUsageFlags.h>
#include <Husky/Graphics/ResourceStorageMode.h>

namespace Husky::Graphics
{
    struct TextureCreateInfo
    {
        TextureType textureType = TextureType::Texture2D;
        Format format = Format::Undefined;
        int32 width = 0;
        int32 height = 0;
        int32 depth = 1;
        int32 mipmapLevelCount = 1;
        int32 arrayLength = 1;
        TextureUsageFlags usage = TextureUsageFlags::Unknown;
        ResourceStorageMode storageMode = ResourceStorageMode::DeviceLocal;
    };
}
