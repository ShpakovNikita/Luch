#pragma once

#include <Husky/Vulkan/Buffer.h>

namespace Husky::Vulkan
{
    class VertexBuffer
    {
        friend class GraphicsDevice;
    public:
        VertexBuffer() = default;
        VertexBuffer(VertexBuffer&& other) = default;
        VertexBuffer& operator=(VertexBuffer&& other) = default;
        ~VertexBuffer() = default;

        inline Buffer& GetUnderlyingBuffer() { return buffer; }
        inline int32 GetVertexSize() { return vertexSize; }
        inline int32 GetVertexCount() { return vertexCount; }
    private:
        inline VertexBuffer(Buffer&& aBuffer, int32 aVertexSize, int32 aVertexCount)
            : buffer(std::forward<Buffer>(aBuffer))
            , vertexSize(aVertexSize)
            , vertexCount(aVertexCount)
        {
        }

        Buffer buffer;
        int32 vertexSize = 0;
        int32 vertexCount = 0;
    };
}
