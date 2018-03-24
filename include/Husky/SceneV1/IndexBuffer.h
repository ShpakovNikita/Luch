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
            const BufferSource& aBufferSource,
            IndexType aIndexType,
            int32 aByteOffset,
            int32 aByteLength)
            : bufferSource(aBufferSource)
            , indexType(aIndexType)
            , byteOffset(aByteOffset)
            , byteLength(aByteLength)
        {
        }
    private:
        BufferSource bufferSource;

        int32 byteOffset = 0;
        int32 byteLength = 0;

        IndexType indexType;

        Vector<uint8> hostBuffer;
        RefPtr<Vulkan::IndexBufferObject> deviceBuffer;
    };
}
