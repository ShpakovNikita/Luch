#pragma once

#include <Husky/Graphics/Surface.h>
#include <Husky/Metal/MetalForwards.h>
#include <Husky/RefPtr.h>

namespace Husky::Metal
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
