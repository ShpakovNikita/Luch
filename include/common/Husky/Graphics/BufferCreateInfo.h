#pragma once

#include <Husky/Graphics/GraphicsCreateInfo.h>
#include <Husky/Graphics/ResourceStorageMode.h>
#include <Husky/Graphics/BufferUsageFlags.h>

namespace Husky::Graphics
{
    struct BufferCreateInfo
    {
        int32 length = 0;
        ResourceStorageMode storageMode = ResourceStorageMode::Shared;
        BufferUsageFlags usage = BufferUsageFlags::Unknown;
    };
}
