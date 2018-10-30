#pragma once

#include <Husky/Types.h>
#include <Husky/RefPtr.h>
#include <Husky/BaseObject.h>
#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/SceneV1/BufferSource.h>

namespace Husky::SceneV1
{
    class Buffer : public BaseObject
    {
    public:
        Buffer() = default;
        Buffer(const BufferSource& source);
        ~Buffer() override;

        inline Vector<Byte> GetHostBuffer() { return hostBuffer; }
        inline RefPtr<Graphics::Buffer> GetDeviceBuffer() { return deviceBuffer; }

        void SetHostBuffer(Vector<Byte> buffer);
        void ReadToHost();
        void ReleaseHostBuffer();

        bool UploadToDevice(Graphics::GraphicsDevice* device);
        void ReleaseDeviceBuffer();

        bool IsResidentOnHost() const;
        bool IsResidentOnDevice() const;
    private:
        Optional<BufferSource> source;
        Vector<Byte> hostBuffer;
        RefPtr<Graphics::Buffer> deviceBuffer;
        bool residentOnHost = false;
    };
}
