#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/Graphics/GraphicsResultValue.h>

namespace Husky::Graphics
{
    class PhysicalDevice : public BaseObject
    {
    public:
        ~PhysicalDevice() = 0 {};

        virtual GraphicsResult<GraphicsDevice> CreateGraphicsDevice() = 0;
    };
}
