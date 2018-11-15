#pragma once

#include <Luch/Graphics/Surface.h>
#include <Luch/Metal/MetalForwards.h>
#include <Luch/RefPtr.h>

namespace Luch::Metal
{
    using namespace Graphics;

    class MetalSurface : public Surface
    {
        friend class MetalGraphicsDevice;
    public:
        MetalSurface(void* layer);
    private:
        void* layer;
    };
}
