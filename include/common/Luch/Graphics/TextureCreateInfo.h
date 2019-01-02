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

        friend bool operator==(const TextureCreateInfo& left, const TextureCreateInfo& right);
    };

    inline bool operator==(const TextureCreateInfo& left, const TextureCreateInfo& right)
    {
        return left.textureType == right.textureType
            && left.format == right.format
            && left.width == right.width
            && left.height == right.height
            && left.depth == right.depth
            && left.mipmapLevelCount == right.mipmapLevelCount
            && left.arrayLength == right.arrayLength
            && left.usage == right.usage
            && left.storageMode == right.storageMode;
    }
}

namespace std
{
    template<>
    struct hash<Luch::Graphics::TextureCreateInfo>
    {
        size_t operator()(const Luch::Graphics::TextureCreateInfo& ci) const
        {
            auto h0 = std::hash<int32_t>()(static_cast<int32_t>(ci.textureType));
            auto h1 = std::hash<int32_t>()(static_cast<int32_t>(ci.format));
            auto h2 = std::hash<int32_t>()(static_cast<int32_t>(ci.width));
            auto h3 = std::hash<int32_t>()(static_cast<int32_t>(ci.height));
            auto h4 = std::hash<int32_t>()(static_cast<int32_t>(ci.depth));
            auto h5 = std::hash<int32_t>()(static_cast<int32_t>(ci.mipmapLevelCount));
            auto h6 = std::hash<int32_t>()(static_cast<int32_t>(ci.arrayLength));
            auto h7 = std::hash<int32_t>()(static_cast<int32_t>(ci.usage));
            auto h8 = std::hash<int32_t>()(static_cast<int32_t>(ci.storageMode));

            return h0
                ^ (h1 << 4)
                ^ (h2 << (4 + 6))
                ^ (h3 << (4 + 6 + 12))
                ^ (h4 << (4 + 6 + 12 + 12))
                ^ (h5 << (4 + 6 + 12 + 12 + 2))
                ^ (h5 << (4 + 6 + 12 + 12 + 2 + 2))
                ^ (h6 << (4 + 6 + 12 + 12 + 2 + 2 + 4))
                ^ (h7 << (4 + 6 + 12 + 12 + 2 + 2 + 4 + 5))
                ^ (h8 << (4 + 6 + 12 + 12 + 2 + 2 + 4 + 5 + 2));
        }
    };
}