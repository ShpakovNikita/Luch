#include <Luch/Metal/MetalPhysicalDevice.h>
#include <Luch/Metal/MetalGraphicsDevice.h>

namespace Luch::Metal
{
    constexpr Array<Format, 3> DepthFormats = 
    {
        Format::D32SfloatS8Uint,
        Format::D24UnormS8Uint,
        Format::D16UnormS8Uint,
    };

    MetalPhysicalDevice::MetalPhysicalDevice(mtlpp::Device aDevice)
        : device(aDevice)
    {
        for(Format format : DepthFormats)
        {
            if(format != Format::D24UnormS8Uint || device.IsDepth24Stencil8PixelFormatSupported())
            {
                capabilities.supportedDepthFormats.push_back(format);
            }
        }

#if LUCH_PLATFORM_IOS
        if(@available(iOS 11.0, *))
        {
            capabilities.hasTileBasedArchitecture = true;
        }
#endif
    }

    GraphicsResultValue<RefPtrVector<MetalPhysicalDevice>> MetalPhysicalDevice::EnumeratePhysicalDevices()
    {
        RefPtrVector<MetalPhysicalDevice> mtlDevices;
#if LUCH_PLATFORM_MACOS
        auto devices = mtlpp::Device::CopyAllDevices();
        int32 deviceCount = devices.GetSize();
#elif LUCH_PLATFORM_IOS
        Array<mtlpp::Device, 1> devices;
        devices[0] = mtlpp::Device::CreateSystemDefaultDevice();
        int32 deviceCount = 1;
#endif
        mtlDevices.reserve(deviceCount);

        for(int32 i = 0; i < deviceCount; i++)
        {
            mtlDevices.push_back(MakeRef<MetalPhysicalDevice>(devices[i]));
        }

        return { GraphicsResult::Success, mtlDevices };
    }

    GraphicsResultRefPtr<GraphicsDevice> MetalPhysicalDevice::CreateGraphicsDevice()
    {
        return { GraphicsResult::Success, MakeRef<MetalGraphicsDevice>(this, device) };
    }
}
