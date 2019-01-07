#pragma once

#include <Luch/BaseObject.h>

namespace Luch::Vulkan
{
    class VulkanInstance : public BaseObject
    {
    public:
        VulkanInstance();
        ~VulkanInstance() override;

// GetInstance
    private:
        void Destroy();
    };
}
