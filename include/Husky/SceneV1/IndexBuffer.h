#pragma once

#include <Husky/RefPtr.h>
#include <Husky/BaseObject.h>
#include <Husky/Vulkan/Forwards.h>
#include <Husky/SceneV1/Forwards.h>
#include <Husky/SceneV1/BufferSource.h>
#include <Husky/IndexType.h>

namespace Husky::SceneV1
{
    class IndexBuffer : public BaseObject
    {
    public:
        IndexBuffer(
            Vector<uint8>&& hostBuffer,
            IndexType indexType,
            int32 count,
            int32 byteOffset,
            int32 byteLength);

        ~IndexBuffer();

        inline IndexType GetIndexType() const { return indexType; }
        inline int32 GetIndexCount() const { return count; }
        inline int32 GetByteOffset() const { return byteOffset; }
        inline const Vector<uint8>& GetHostBuffer() const { return hostBuffer; }
        inline const RefPtr<Vulkan::Buffer>& GetDeviceBuffer() const { return deviceBuffer; }
    private:
        BufferSource bufferSource;

        IndexType indexType = IndexType::UInt16;
        int32 count = 0;

        int32 byteOffset = 0;
        int32 byteLength = 0;

        Vector<uint8> hostBuffer;
        RefPtr<Vulkan::Buffer> deviceBuffer;
    };
}