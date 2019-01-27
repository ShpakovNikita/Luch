#pragma once

#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/ResourceStorageMode.h>
#include <Luch/Graphics/BufferUsageFlags.h>

namespace Luch::Graphics
{
    struct BufferCreateInfo
    {
        int32 length = 0;
        ResourceStorageMode storageMode = ResourceStorageMode::Shared;
        BufferUsageFlags usage = BufferUsageFlags::Unknown;

        friend bool operator==(const BufferCreateInfo& left, const BufferCreateInfo& right);
    };

    inline bool operator==(const BufferCreateInfo& left, const BufferCreateInfo& right)
    {
        return left.length == right.length
            && left.storageMode == right.storageMode
            && left.usage == right.usage;
    }
}


namespace std
{
    template<>
    struct hash<Luch::Graphics::BufferCreateInfo>
    {
        size_t operator()(const Luch::Graphics::BufferCreateInfo& ci) const
        {
            auto h0 = std::hash<int32_t>()(static_cast<int32_t>(ci.length));
            auto h1 = std::hash<int32_t>()(static_cast<int32_t>(ci.storageMode));
            auto h2 = std::hash<int32_t>()(static_cast<int32_t>(ci.usage));

            return h0
                ^ (h1 << 32)
                ^ (h2 << (32 + 4));
        }
    };
}
