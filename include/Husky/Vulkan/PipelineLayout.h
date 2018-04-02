#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;

    class PipelineLayout : public BaseObject
    {
        friend class GraphicsDevice;
    public:
        PipelineLayout(GraphicsDevice* device, vk::PipelineLayout pipelineLayout);

        PipelineLayout(PipelineLayout&& other) = delete;
        PipelineLayout(const PipelineLayout& other) = delete;
        PipelineLayout& operator=(const PipelineLayout& other) = delete;
        PipelineLayout& operator=(PipelineLayout&& other) = delete;

        ~PipelineLayout() override;

        inline vk::PipelineLayout GetPipelineLayout() { return pipelineLayout; }
    private:
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::PipelineLayout pipelineLayout;
    };
}
