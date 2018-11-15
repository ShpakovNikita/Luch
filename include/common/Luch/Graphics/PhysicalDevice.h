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

        //virtual Vector<Format> GetSupportedColorFormats(const Vector<Format>& formats) = 0;
        virtual Vector<Format> GetSupportedDepthStencilFormats(const Vector<Format>& formats) const = 0;

        virtual GraphicsResultRefPtr<GraphicsDevice> CreateGraphicsDevice() = 0;
    };

    inline PhysicalDevice::~PhysicalDevice() {}
}