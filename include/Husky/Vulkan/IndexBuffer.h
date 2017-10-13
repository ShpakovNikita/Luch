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
        inline int32 GetIndexCount() const { return indexCount; }

        inline void SetIndexType(IndexType newIndexType)
        {
            auto currentSize = indexCount * IndexSize(indexType);
            auto newCount = currentSize / IndexSize(newIndexType);

            indexCount = newCount;
            indexType = newIndexType;
        }

        inline Buffer& GetUnderlyingBuffer() { return buffer; }
    private:
        inline IndexBuffer(Buffer&& aBuffer, IndexType aIndexType, int32 aIndexCount)
            : buffer(std::forward<Buffer>(aBuffer))
            , indexType(aIndexType)
            , indexCount(aIndexCount)
        {
        }

        Buffer buffer;
        int32 indexCount = 0;
        IndexType indexType;
    };
}
