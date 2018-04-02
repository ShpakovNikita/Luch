#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{
    class RenderPass : public BaseObject
    {
        friend class GraphicsDevice;
    public:
        RenderPass(GraphicsDevice* device, vk::RenderPass renderPass, int32 attachmentCount);

        RenderPass(const RenderPass& other) = delete;
        RenderPass(RenderPass&& other) = delete;
        RenderPass& operator=(const RenderPass& other) = delete;
        RenderPass& operator=(RenderPass&& other) = delete;

        ~RenderPass() override;

        inline vk::RenderPass GetRenderPass() { return renderPass; }
        inline int32 GetAttachmentCount() const { return attachmentCount; }
    private:
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::RenderPass renderPass;
        int32 attachmentCount = 0;
    };
}
