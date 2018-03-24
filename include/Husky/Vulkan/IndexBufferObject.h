#pragma once

#include <Husky/RefPtr.h>
#include <Husky/IndexType.h>
#include <Husky/Vulkan/Buffer.h>

namespace Husky::Vulkan
{
    class IndexBufferObject : public BaseObject
    {
    public:
        IndexBufferObject(
            const RefPtr<BufferObject>& aBuffer,
            IndexType aIndexType,
            int32 aIndexCount)
            : buffer(aBuffer)
            , indexType(aIndexType)
            , indexCount(aIndexCount)
        {
        }

        ~IndexBufferObject() = default;

        inline IndexType GetIndexType() const { return indexType; }
        inline int32 GetIndexCount() const { return indexCount; }

        inline void SetIndexType(IndexType newIndexType)
        {
            auto currentSize = indexCount * IndexSize(indexType);
            auto newCount = currentSize / IndexSize(newIndexType);

            indexCount = newCount;
            indexType = newIndexType;
        }

        inline Buffer* GetUnderlyingBuffer() { return buffer->GetBuffer(); }
    private:
        RefPtr<BufferObject> buffer;
        int32 indexCount = 0;
        IndexType indexType;
    };
}
