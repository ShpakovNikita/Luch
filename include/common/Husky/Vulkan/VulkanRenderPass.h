#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Forwards.h>

namespace Husky::Vulkan
{
    class VulkanRenderPass : public BaseObject
    {
        friend class VulkanGraphicsDevice;
    public:
        VulkanRenderPass(
            VulkanGraphicsDevice* device,
            vk::RenderPass renderPass,
            int32 attachmentCount);
        ~RenderPass() override;

        inline vk::RenderPass GetRenderPass() { return renderPass; }
        inline int32 GetAttachmentCount() const { return attachmentCount; }
    private:
        void Destroy();

        VulkanGraphicsDevice* device = nullptr;
        vk::RenderPass renderPass;
        int32 attachmentCount = 0;
    };
}
