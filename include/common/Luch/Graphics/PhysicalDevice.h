#pragma once

#include <Luch/BaseObject.h>
#include <Luch/Graphics/Format.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/GraphicsResultValue.h>

namespace Luch::Graphics
{
    class PhysicalDevice : public BaseObject
    {
    public:
        virtual ~PhysicalDevice() = 0;

        virtual GraphicsResultRefPtr<GraphicsDevice> CreateGraphicsDevice() = 0;
        virtual const PhysicalDeviceCapabilities& GetCapabilities() = 0;
    };

    inline PhysicalDevice::~PhysicalDevice() {}
}
