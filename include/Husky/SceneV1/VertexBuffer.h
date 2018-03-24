#pragma once

#include <Husky/RefPtr.h>
#include <Husky/BaseObject.h>
#include <Husky/SceneV1/Forwards.h>
#include <Husky/Vulkan/VertexBufferObject.h>
#include <Husky/SceneV1/BufferSource.h>

namespace Husky::SceneV1
{
    class VertexBuffer : public BaseObject
    {
    public:
        VertexBuffer(
            const BufferSource& aBufferSource,
            int32 aStride,
            int32 aByteOffset,
            int32 aByteLength)
            : bufferSource(aBufferSource)
            , stride(aStride)
            , byteOffset(aByteOffset)
            , byteLength(aByteLength)
        {
        }
    private:
        BufferSource bufferSource;

        int32 stride = 4;
        int32 byteOffset = 0;
        int32 byteLength = 0;

        Vector<uint8> hostBuffer;
        RefPtr<Vulkan::VertexBufferObject> deviceBuffer;
    };
}