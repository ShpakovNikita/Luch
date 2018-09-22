#pragma once

#include <Husky/Graphics/PhysicalDevice.h>
#include <mtlpp.hpp>

namespace Husky::Metal
{
    using namespace Graphics;

    class MetalPhysicalDevice : public PhysicalDevice
    {
    public:
        MetalPhysicalDevice(mtlpp::Device device);

        GraphicsResultRefPtr<GraphicsDevice> CreateGraphicsDevice() override;
    private:
        mtlpp::Device device;
    };
}

