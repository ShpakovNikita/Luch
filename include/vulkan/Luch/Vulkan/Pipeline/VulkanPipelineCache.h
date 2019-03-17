#pragma once

#include <Luch/BaseObject.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/Common/VulkanForwards.h>

namespace Luch::Vulkan
{
    class VulkanPipelineCache : public BaseObject
    {
        friend class VulkanGraphicsDevice;
    public:
        VulkanPipelineCache(VulkanGraphicsDevice* device, vk::PipelineCache pipelineCache);
        ~VulkanPipelineCache() override;

        vk::PipelineCache GetPipelineCache() { return pipelineCache; }
    private:
        void Destroy();

        VulkanGraphicsDevice* device = nullptr;
        vk::PipelineCache pipelineCache;
    };
}
