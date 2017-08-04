#pragma once

#include <Husky/Vulkan.h>

namespace Husky
{

class VulkanPhysicalDevice
{
public:
    VulkanPhysicalDevice(VkPhysicalDevice handle);
    
private:
    VkPhysicalDevice handle;
};

}