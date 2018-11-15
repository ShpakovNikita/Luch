#pragma once

#include <Luch/Types.h>
#include <Luch/Graphics/Format.h>
#include <Luch/Graphics/TextureType.h>
#include <Luch/Graphics/TextureUsageFlags.h>
#include <Luch/Graphics/ResourceStorageMode.h>

namespace Luch::Graphics
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
