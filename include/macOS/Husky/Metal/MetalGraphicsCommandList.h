#pragma once

#include <Husky/Graphics/GraphicsCommandList.h>
#include <Husky/Metal/MetalForwards.h>
#include <mtlpp.hpp>

namespace Husky::Metal
{
    class MetalGraphicsCommandList : public GraphicsCommandList
    {
    public:
        MetalGraphicsCommandList(
            MetalCommandQueue* queue,
            mtlpp::CommandBuffer commandBuffer);

        void Begin() override;
        void End() override;

        
    private:
        mtlpp::CommandBuffer commandBuffer;
    };
}
