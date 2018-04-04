#pragma once

#include <Husky/RefPtr.h>
#include <Husky/BaseObject.h>
#include <Husky/SceneV1/Forwards.h>
#include <Husky/SceneV1/BufferSource.h>
#include <Husky/Vulkan/Forwards.h>

namespace Husky::SceneV1
{
    class VertexBuffer : public BaseObject
    {
    public:
        VertexBuffer(
            Vector<uint8>&& aHostBuffer,
            int32 aStride,
            int32 aByteOffset,
            int32 aByteLength);

        ~VertexBuffer();

        inline int32 GetStride() const { return stride; }
        inline int32 GetByteOffset() const { return byteOffset; }
        inline const Vector<uint8>& GetHostBuffer() const { return hostBuffer; }
        inline const RefPtr<Vulkan::Buffer>& GetDeviceBuffer() const { return deviceBuffer; }
    private:
        int32 stride = 4;
        int32 byteOffset = 0;
        int32 byteLength = 0;

        Vector<uint8> hostBuffer;
        RefPtr<Vulkan::Buffer> deviceBuffer;
    };
}
