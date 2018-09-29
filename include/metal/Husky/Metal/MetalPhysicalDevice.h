#pragma once

#include <Husky/Graphics/PhysicalDevice.h>
#include <Husky/Graphics/GraphicsResultValue.h>
#include <mtlpp.hpp>

namespace Husky::Metal
{
    using namespace Graphics;

    class MetalPhysicalDevice : public PhysicalDevice
    {
    public:
        static GraphicsResultValue<RefPtrVector<MetalPhysicalDevice>> EnumeratePhysicalDevices();

        MetalPhysicalDevice(mtlpp::Device device);

        Vector<Format> GetSupportedDepthStencilFormats(const Vector<Format>& formats) const override;
        GraphicsResultRefPtr<GraphicsDevice> CreateGraphicsDevice() override;
    private:
        mtlpp::Device device;
    };
}

