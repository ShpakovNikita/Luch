#include <Husky/SceneV1/Buffer.h>
#include <Husky/FileStream.h>
#include <Husky/UniquePtr.h>
#include <Husky/Vulkan/GraphicsDevice.h>
#include <Husky/Vulkan/DeviceBuffer.h>

namespace Husky::SceneV1
{
    Buffer::Buffer(const BufferSource& aSource)
        : source(aSource)
    {
    }

    Buffer::~Buffer() = default;

    void Buffer::ReadToHost()
    {
        UniquePtr<FileStream> stream = MakeUnique<FileStream>(source.root + "/" + source.filename, FileOpenModes::Read);
        hostBuffer.resize(source.byteLength);

        stream->Read(hostBuffer.data(), source.byteLength, 1);
    }

    void Buffer::ReleaseHostBuffer()
    {
        hostBuffer.clear();
        hostBuffer.shrink_to_fit();
    }

    bool Buffer::UploadToDevice(Vulkan::GraphicsDevice* device)
    {
        auto[createBufferResult, createdBuffer] = device->CreateBuffer(
            source.byteLength,
            device->GetQueueIndices()->graphicsQueueFamilyIndex,
            vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer,
            true);

        if (createBufferResult != vk::Result::eSuccess)
        {
            return false;
        }

        auto[mapResult, mappedMemory] = createdBuffer->MapMemory(source.byteLength, 0);
        if (mapResult != vk::Result::eSuccess)
        {
            return false;
        }

        memcpy(mappedMemory, hostBuffer.data(), source.byteLength);

        createdBuffer->UnmapMemory();

        deviceBuffer = createdBuffer;

        return true;
    }
}
