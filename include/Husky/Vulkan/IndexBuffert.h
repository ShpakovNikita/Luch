#pragma once

#include <Husky/IndexType.h>
#include <Husky/Vulkan/Buffer.h>

namespace Husky::Vulkan
{
    class IndexBuffer
    {
    public:
        IndexBuffer(
            const RefPtr<Buffer>& aBuffer,
            IndexType aIndexType,
            int32 aIndexCount)
            : buffer(aBuffer)
            , indexType(aIndexType)
            , indexCount(aIndexCount)
        {
        }

        inline IndexType GetIndexType() const { return indexType; }
        inline int32 GetIndexCount() const { return indexCount; }

        inline void SetIndexType(IndexType newIndexType)
        {
            auto currentSize = indexCount * IndexSize(indexType);
            auto newCount = currentSize / IndexSize(newIndexType);

            indexCount = newCount;
            indexType = newIndexType;
        }

        inline Buffer* GetUnderlyingBuffer() { return buffer.Get(); }
    private:
        RefPtr<Buffer> buffer;
        int32 indexCount = 0;
        IndexType indexType;
    };
}
