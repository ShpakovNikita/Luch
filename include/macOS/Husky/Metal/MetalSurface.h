#pragma once

#include <Husky/BaseObject.h>
#include <Husky/RefPtr.h>

namespace Husky::Metal
{
    class MetalSurface : public BaseObject
    {
    public:
        MetalSurface(void* view);
        ~MetalSurface();
    private:
        void* view = nullptr;
    };
}
