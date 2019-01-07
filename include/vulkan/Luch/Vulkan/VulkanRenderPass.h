#pragma once

#include <Luch/BaseObject.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/VulkanForwards.h>

namespace Luch::Vulkan
{
    class VulkanRenderPass : public BaseObject
    {
        friend class VulkanGraphicsDevice;
    public:
        VulkanRenderPass(
            VulkanGraphicsDevice* device,
            vk::RenderPass renderPass,
            int32 attachmentCount);
        ~VulkanRenderPass() override;

        inline vk::RenderPass GetRenderPass() { return renderPass; }
        inline int32 GetAttachmentCount() const { return attachmentCount; }
    private:
        void Destroy();

        VulkanGraphicsDevice* device = nullptr;
        vk::RenderPass renderPass;
        int32 attachmentCount = 0;
    };
}
