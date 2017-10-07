#pragma once

#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;

    class PipelineLayout
    {
        friend class GraphicsDevice;
    public:
        PipelineLayout() = default;

        PipelineLayout(PipelineLayout&& other);
        PipelineLayout& operator=(PipelineLayout&& other);

        ~PipelineLayout();

        vk::PipelineLayout GetPipelineLayout() { return pipelineLayout; }
    private:
        PipelineLayout(GraphicsDevice* device, vk::PipelineLayout pipelineLayout);
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::PipelineLayout pipelineLayout;
    };
}
