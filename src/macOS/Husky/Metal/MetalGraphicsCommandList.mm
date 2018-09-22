#include <Husky/Metal/MetalGraphicsCommandList.h>
#include <Husky/Metal/MetalCommandQueue.h>

namespace Husky::Metal
{
    MetalGraphicsCommandList::MetalGraphicsCommandList(
        MetalCommandQueue* queue,
        mtlpp::CommandBuffer commandBuffer)
        : GraphicsCommandList(queue->GetGraphicsDevice())
    {
    }

    void MetalGraphicsCommandList::Begin()
    {
    }

    void MetalGraphicsCommandList::End()
    {
        commandBuffer.f
    }
}
