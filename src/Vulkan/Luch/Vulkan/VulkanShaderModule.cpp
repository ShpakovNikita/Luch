#include <Luch/Vulkan/VulkanShaderModule.h>
#include <Luch/Vulkan/VulkanGraphicsDevice.h>

namespace Luch::Vulkan
{
    VulkanShaderModule::VulkanShaderModule(
        VulkanGraphicsDevice* aDevice,
        vk::ShaderModule aModule)
        : device(aDevice)
        , module(aModule)
    {
    }

    VulkanShaderModule::~VulkanShaderModule()
    {
        Destroy();
    }

    void VulkanShaderModule::Destroy()
    {
        if (device)
        {
            device->DestroyShaderModule(this);
        }
    }
}
