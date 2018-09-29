#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Graphics/Format.h>
#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/Graphics/GraphicsResultValue.h>

namespace Husky::Graphics
{
    class PhysicalDevice : public BaseObject
    {
    public:
        virtual ~PhysicalDevice() = 0;

        //virtual Vector<Format> GetSupportedColorFormats(const Vector<Format>& formats) = 0;
        virtual Vector<Format> GetSupportedDepthStencilFormats(const Vector<Format>& formats) const = 0;

        virtual GraphicsResultRefPtr<GraphicsDevice> CreateGraphicsDevice() = 0;
    };

    inline PhysicalDevice::~PhysicalDevice() {}
}
