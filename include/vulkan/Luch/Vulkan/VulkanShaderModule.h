#pragma once

#include <Luch/BaseObject.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/VulkanForwards.h>

namespace Luch::Vulkan
{
    class VulkanShaderModule : public BaseObject
    {
        friend class VulkanGraphicsDevice;
    public:
        VulkanShaderModule(VulkanGraphicsDevice* device, vk::ShaderModule module);
        ~VulkanShaderModule() override;

        inline vk::ShaderModule GetModule() { return module; }
    private:
        void Destroy();

        VulkanGraphicsDevice* device = nullptr;
        vk::ShaderModule module;
    };
}
