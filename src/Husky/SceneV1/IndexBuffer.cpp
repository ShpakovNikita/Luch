#include <Husky/SceneV1/IndexBuffer.h>
#include <Husky/Vulkan/Buffer.h>

namespace Husky::SceneV1
{
    IndexBuffer::IndexBuffer(
        Vector<uint8>&& aHostBuffer,
        IndexType aIndexType,
        int32 aCount,
        int32 aByteOffset,
        int32 aByteLength)
        : hostBuffer(move(aHostBuffer))
        , indexType(aIndexType)
        , count(aCount)
        , byteOffset(aByteOffset)
        , byteLength(aByteLength)
    {
    }

    IndexBuffer::~IndexBuffer() = default;
}
