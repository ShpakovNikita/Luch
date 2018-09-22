#include <Husky/Metal/MetalPhysicalDevice.h>
#include <Husky/Metal/MetalGraphicsDevice.h>

namespace Husky::Metal
{
    MetalPhysicalDevice::MetalPhysicalDevice(mtlpp::Device aDevice)
        : device(aDevice)
    {
    }

    GraphicsResultRefPtr<GraphicsDevice> MetalPhysicalDevice::CreateGraphicsDevice()
    {
        return { GraphicsResult::Success, MakeRef<MetalGraphicsDevice>(this, device) };
    }
}
