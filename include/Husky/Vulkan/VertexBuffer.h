#pragma once

#include <Husky/RefPtr.h>
#include <Husky/Vulkan/Buffer.h>

namespace Husky::Vulkan
{
    class VertexBuffer
    {
        friend class GraphicsDevice;
    public:
        VertexBuffer(
            const RefPtr<Buffer>& aBuffer,
            int32 aVertexSize,
            int32 aVertexCount)
            : buffer(aBuffer)
            , vertexSize(aVertexSize)
            , vertexCount(aVertexCount)
        {
        }

        inline Buffer* GetUnderlyingBuffer() { return buffer.Get(); }
        inline int32 GetVertexSize() { return vertexSize; }
        inline int32 GetVertexCount() { return vertexCount; }
    private:
        RefPtr<Buffer> buffer;
        int32 vertexSize = 0;
        int32 vertexCount = 0;
    };
}
