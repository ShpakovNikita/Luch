#include <Husky/SceneV1/Buffer.h>
#include <Husky/FileStream.h>
#include <Husky/UniquePtr.h>
#include <Husky/Graphics/GraphicsDevice.h>
#include <Husky/Graphics/Buffer.h>
#include <Husky/Graphics/BufferCreateInfo.h>

namespace Husky::SceneV1
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

        HUSKY_ASSERT(source.has_value());

        UniquePtr<FileStream> stream = MakeUnique<FileStream>(source->root + "/" + source->filename, FileOpenModes::Read);
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
