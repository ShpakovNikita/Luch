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
    };
}
