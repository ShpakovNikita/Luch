#include <Luch/SceneV1/Buffer.h>
#include <Luch/FileStream.h>
#include <Luch/UniquePtr.h>
#include <Luch/Graphics/GraphicsDevice.h>
#include <Luch/Graphics/Buffer.h>
#include <Luch/Graphics/BufferCreateInfo.h>

namespace Luch::SceneV1
{
    Buffer::Buffer(const BufferSource& aSource)
        : source(aSource)
    {
    }

    Buffer::~Buffer() = default;

    void Buffer::SetHostBuffer(Vector<Byte> buffer)
    {
        hostBuffer = std::move(buffer);
        residentOnHost = true;
    }

    void Buffer::ReadToHost()
    {
        if(residentOnHost)
        {
            return;
        }

        LUCH_ASSERT(source.has_value());

        UniquePtr<FileStream> stream = MakeUnique<FileStream>(source->root + source->filename, FileOpenModes::Read);
        hostBuffer.resize(source->byteLength);

        stream->Read(hostBuffer.data(), source->byteLength, 1);

        residentOnHost = true;
    }

    void Buffer::ReleaseHostBuffer()
    {
        hostBuffer.clear();
        hostBuffer.shrink_to_fit();
        residentOnHost = false;
    }

    bool Buffer::UploadToDevice(Graphics::GraphicsDevice* device)
    {
        Graphics::BufferCreateInfo ci;
        ci.length = hostBuffer.size();
        ci.usage = Graphics::BufferUsageFlags::VertexBuffer | Graphics::BufferUsageFlags::IndexBuffer;
        ci.storageMode = Graphics::ResourceStorageMode::Shared;

        auto[createBufferResult, createdBuffer] = device->CreateBuffer(
            ci,
            hostBuffer.data());

        if (createBufferResult != Graphics::GraphicsResult::Success)
        {
            return false;
        }

        deviceBuffer = createdBuffer;

        return true;
    }

    bool Buffer::IsResidentOnHost() const
    {
        return residentOnHost;
    }

    bool Buffer::IsResidentOnDevice() const
    {
        return deviceBuffer != nullptr;
    }
}
