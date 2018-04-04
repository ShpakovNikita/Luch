#include <Husky/SceneV1/VertexBuffer.h>
#include <Husky/Vulkan/Buffer.h>

namespace Husky::SceneV1
{
    VertexBuffer::VertexBuffer(
        Vector<uint8>&& aHostBuffer,
        int32 aStride,
        int32 aByteOffset,
        int32 aByteLength)
        : hostBuffer(aHostBuffer)
        , stride(aStride)
        , byteOffset(aByteOffset)
        , byteLength(aByteLength)
    {
    }

    VertexBuffer::~VertexBuffer() = default;
}
