#pragma once

#include <Husky/BaseObject.h>
#include <Husky/RefPtr.h>

namespace Husky::Metal
{
    class MetalDevice : public BaseObject
    {
    public:
        MetalDevice(void* device);
        ~MetalDevice();
    private:
        void* device = nullptr;
    }
}
