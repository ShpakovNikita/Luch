#pragma once

#include <Luch/BaseObject.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/Common/VulkanForwards.h>
#include <Luch/Graphics/RenderPass.h>
#include <Luch/Graphics/RenderPassCreateInfo.h>

using namespace Luch::Graphics;

namespace Luch::Vulkan
{
    class VulkanRenderPass : public RenderPass
    {
        friend class VulkanGraphicsDevice;
    public:
        VulkanRenderPass(
            VulkanGraphicsDevice* device,
            vk::RenderPass renderPass,
            int32 attachmentCount);
        ~VulkanRenderPass() override;

        const RenderPassCreateInfo& GetCreateInfo() const override;

        inline vk::RenderPass GetRenderPass() { return renderPass; }
        inline int32 GetAttachmentCount() const { return attachmentCount; }
    private:
        void Destroy();

        VulkanGraphicsDevice* device = nullptr;
        RenderPassCreateInfo createInfoUnused;
        vk::RenderPass renderPass;
        int32 attachmentCount = 0;
    };
}
