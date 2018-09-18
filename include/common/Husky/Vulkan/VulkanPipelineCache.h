#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Forwards.h>

namespace Husky::Vulkan
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
