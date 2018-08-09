#pragma once

#include <Husky/Types.h>
#include <Husky/RefPtr.h>
#include <Husky/BaseObject.h>
#include <Husky/Vulkan/Forwards.h>
#include <Husky/SceneV1/BufferSource.h>

namespace Husky::SceneV1
{
    class Buffer : public BaseObject
    {
    public:
        Buffer(const BufferSource& source);
        ~Buffer() override;

        inline Vector<uint8> GetHostBuffer() { return hostBuffer; }
        inline RefPtr<Vulkan::DeviceBuffer> GetDeviceBuffer() { return deviceBuffer; }

        void ReadToHost();
        void ReleaseHostBuffer();

        bool UploadToDevice(Vulkan::GraphicsDevice* device);
        void ReleaseDeviceBuffer();
    private:
        BufferSource source;
        Vector<uint8> hostBuffer;
        RefPtr<Vulkan::DeviceBuffer> deviceBuffer;
    };
}
