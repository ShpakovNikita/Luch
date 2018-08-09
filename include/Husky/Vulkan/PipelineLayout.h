#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Forwards.h>

namespace Husky::Vulkan
{
    class PipelineLayout : public BaseObject
    {
        friend class GraphicsDevice;
    public:
        PipelineLayout(GraphicsDevice* device, vk::PipelineLayout pipelineLayout);
        ~PipelineLayout() override;

        inline vk::PipelineLayout GetPipelineLayout() { return pipelineLayout; }
    private:
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::PipelineLayout pipelineLayout;
    };
}
