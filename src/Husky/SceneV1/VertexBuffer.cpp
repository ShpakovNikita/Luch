#include <Husky/SceneV1/VertexBuffer.h>
#include <Husky/Vulkan/Buffer.h>
#include <Husky/Vulkan/GraphicsDevice.h>

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

    bool VertexBuffer::UploadToDevice(const RefPtr<Vulkan::GraphicsDevice>& device)
    {
        auto[createVertexBufferResult, createdVertexBuffer] = device->CreateBuffer(
            byteLength,
            device->GetQueueIndices()->graphicsQueueFamilyIndex,
            vk::BufferUsageFlagBits::eVertexBuffer,
            true);

        if (createVertexBufferResult != vk::Result::eSuccess)
        {
            // TODO
            return false;
        }

        {
            auto[mapVerticesResult, verticesMemory] = createdVertexBuffer->MapMemory(byteLength, 0);
            if (mapVerticesResult != vk::Result::eSuccess)
            {
                return false;
            }

            memcpy(verticesMemory, hostBuffer.data() + byteOffset, byteLength);

            createdVertexBuffer->UnmapMemory();
        }

        return true;
    }
}
