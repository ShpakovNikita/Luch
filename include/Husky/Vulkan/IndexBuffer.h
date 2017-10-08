#pragma once

#include <Husky/IndexType.h>
#include <Husky/Vulkan/Buffer.h>

namespace Husky::Vulkan
{
    class IndexBuffer
    {
        friend class GraphicsDevice;
    public:
        IndexBuffer() = default;
        IndexBuffer(IndexBuffer&& other) = default;
        IndexBuffer& operator=(IndexBuffer&& other) = default;
        ~IndexBuffer() = default;

        inline IndexType GetIndexType() const { return indexType; }
        inline Buffer& GetUnderlyingBuffer() { return buffer; }
    private:
        inline IndexBuffer(Buffer&& aBuffer, int32 aIndexCount, IndexType aIndexType)
            : buffer(std::forward<Buffer>(aBuffer))
            , indexCount(aIndexCount)
            , indexType(aIndexType)
        {
        }

        Buffer buffer;
        int32 indexCount;
        IndexType indexType;
    };
}
