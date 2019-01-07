#include <Luch/Vulkan/VulkanInstance.h>

namespace Luch::Vulkan
{
    VulkanInstance::VulkanInstance()
    {
    }

    VulkanInstance::~VulkanInstance()
    {
        Destroy();
    }

    void VulkanInstance::Destroy()
    {
        // cleanup if needed
    }
}
