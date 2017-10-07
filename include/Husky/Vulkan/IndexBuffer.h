#pragma once

#include <Husky/IndexType.h>
#include <Husky/Vulkan/Buffer.h>

namespace Husky::Vulkan
{
    class IndexBuffer
    {
    public:
        IndexBuffer() = default;

        inline IndexType GetIndexType() { return indexType; }
    private:
        Buffer buffer;
        IndexType indexType;
    };
}
