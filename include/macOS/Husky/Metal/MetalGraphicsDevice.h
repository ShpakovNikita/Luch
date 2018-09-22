#pragma once

#include <Husky/Graphics/GraphicsDevice.h>
#include <mtlpp.hpp>

namespace Husky::Metal
{
    using namespace Graphics;

    class MetalGraphicsDevice : public GraphicsDevice
    {
    public:
        MetalGraphicsDevice(
            PhysicalDevice* physicalDevice,
            mtlpp::Device device);

        PhysicalDevice* GetPhysicalDevice() override { return physicalDevice; }

        GraphicsResultRefPtr<CommandQueue> CreateCommandQueue() override;
    private:
        PhysicalDevice* physicalDevice = nullptr;
        mtlpp::Device device;
    };
}
