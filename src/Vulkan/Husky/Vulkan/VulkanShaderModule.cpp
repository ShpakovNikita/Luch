#include <Husky/Vulkan/VulkanShaderModule.h>
#include <Husky/Vulkan/VulkanGraphicsDevice.h>

namespace Husky::Vulkan
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
