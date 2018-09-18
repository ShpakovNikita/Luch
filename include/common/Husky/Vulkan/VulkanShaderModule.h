#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/VulkanForwards.h>

namespace Husky::Vulkan
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
