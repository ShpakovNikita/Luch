#pragma once

#include <Luch/Graphics/PhysicalDevice.h>
#include <Luch/Graphics/PhysicalDeviceCapabilities.h>
#include <Luch/Graphics/GraphicsResultValue.h>
#include <mtlpp.hpp>

namespace Luch::Metal
{
    using namespace Graphics;

    class MetalPhysicalDevice : public PhysicalDevice
    {
    public:
        static GraphicsResultValue<RefPtrVector<MetalPhysicalDevice>> EnumeratePhysicalDevices();

        MetalPhysicalDevice(mtlpp::Device device);

        const PhysicalDeviceCapabilities& GetCapabilities() override { return capabilities; }
        GraphicsResultRefPtr<GraphicsDevice> CreateGraphicsDevice() override;
    private:
        mtlpp::Device device;
        PhysicalDeviceCapabilities capabilities;
    };
}
