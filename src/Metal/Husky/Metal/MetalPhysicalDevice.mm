#include <Husky/Metal/MetalPhysicalDevice.h>
#include <Husky/Metal/MetalGraphicsDevice.h>

namespace Husky::Metal
{
    MetalPhysicalDevice::MetalPhysicalDevice(mtlpp::Device aDevice)
        : device(aDevice)
    {
    }

    GraphicsResultValue<RefPtrVector<MetalPhysicalDevice>> MetalPhysicalDevice::EnumeratePhysicalDevices()
    {
        auto devices = mtlpp::Device::CopyAllDevices();
        int32 deviceCount = devices.GetSize();

        RefPtrVector<MetalPhysicalDevice> mtlDevices;
        mtlDevices.reserve(deviceCount);

        for(int32 i = 0; i < deviceCount; i++)
        {
            mtlDevices.push_back(MakeRef<MetalPhysicalDevice>(devices[i]));
        }

        return { GraphicsResult::Success, mtlDevices };
    }

    Vector<Format> MetalPhysicalDevice::GetSupportedDepthStencilFormats(const Vector<Format>& formats) const
    {
        Vector<Format> supportedFormats;
        for(Format format : formats)
        {
            if(format != Format::D24UnormS8Uint || device.IsDepth24Stencil8PixelFormatSupported())
            {
                supportedFormats.push_back(format);
            }
        }

        return formats;
    }

    GraphicsResultRefPtr<GraphicsDevice> MetalPhysicalDevice::CreateGraphicsDevice()
    {
        return { GraphicsResult::Success, MakeRef<MetalGraphicsDevice>(this, device) };
    }
}
