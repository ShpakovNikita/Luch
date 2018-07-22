#include <Husky/SceneV1/IndexBuffer.h>
#include <Husky/Vulkan/Buffer.h>
#include <Husky/Vulkan/GraphicsDevice.h>

namespace Husky::SceneV1
{
    IndexBuffer::IndexBuffer(
        SharedPtr<Vector<uint8>> aHostBuffer,
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

    bool IndexBuffer::UploadToDevice(const RefPtr<Vulkan::GraphicsDevice>& device)
    {
        auto[createIndexBufferResult, createdIndexBuffer] = device->CreateBuffer(
            byteLength,
            device->GetQueueIndices()->graphicsQueueFamilyIndex,
            vk::BufferUsageFlagBits::eIndexBuffer,
            true);

        if (createIndexBufferResult != vk::Result::eSuccess)
        {
            // TODO
            return false;
        }

        {
            auto[mapIndicesResult, indicesMemory] = createdIndexBuffer->MapMemory(byteLength, 0);
            if (mapIndicesResult != vk::Result::eSuccess)
            {
                return false;
            }

            memcpy(indicesMemory, hostBuffer->data() + byteOffset, byteLength);

            createdIndexBuffer->UnmapMemory();
        }

        deviceBuffer = createdIndexBuffer;

        return true;
    }
}
