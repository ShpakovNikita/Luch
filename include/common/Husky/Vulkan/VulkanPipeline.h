#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Forwards.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;

    class VulkanPipeline : public BaseObject
    {
        friend class VulkanGraphicsDevice;
    public:
        VulkanPipeline(VulkanGraphicsDevice* device, vk::Pipeline pipeline);
        ~VulkanPipeline() override;

        vk::Pipeline GetPipeline() { return pipeline; }
    private:
        void Destroy();

        VulkanGraphicsDevice* device = nullptr;
        vk::Pipeline pipeline;
    };
}
