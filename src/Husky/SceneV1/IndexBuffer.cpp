#include <Husky/SceneV1/IndexBuffer.h>
#include <Husky/Vulkan/IndexBufferObject.h>

namespace Husky::SceneV1
{
    IndexBuffer::IndexBuffer(
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

    IndexBuffer::~IndexBuffer() = default;
}
