#pragma once

#include <Luch/Metal/MetalForwards.h>

namespace Luch::Metal
{
    class MetalCommandList
    {
        friend class MetalCommandQueue;
    public:
        MetalCommandList(mtlpp::CommandBuffer aCommandBuffer) 
            : commandBuffer(aCommandBuffer)
        {
        }
    protected:
        mtlpp::CommandBuffer commandBuffer;
    };
}
