#pragma once

#include <Luch/Types.h>
#include <Luch/RefPtr.h>
#include <Luch/BaseObject.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/SceneV1/BufferSource.h>

namespace Luch::SceneV1
{
    class Buffer : public BaseObject
    {
    public:
        Buffer() = default;
        Buffer(const BufferSource& source);
        ~Buffer() override;

        inline Vector<Byte>& GetHostBuffer() { return hostBuffer; }
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
