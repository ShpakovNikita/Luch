#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/Graphics/GraphicsResultValue.h>

namespace Husky::Graphics
{
    class PhysicalDevice : public BaseObject
    {
    public:
        virtual ~PhysicalDevice() = 0;

        virtual GraphicsResultRefPtr<GraphicsDevice> CreateGraphicsDevice() = 0;
    };

    inline PhysicalDevice::~PhysicalDevice() {}
}
