#pragma once

#include <Luch/Graphics/CommandPool.h>
#include <Luch/Metal/MetalForwards.h>

namespace Luch::Metal
{
    using namespace Graphics;

    class MetalCommandPool : public CommandPool
    {
    public:
        MetalCommandPool(MetalGraphicsDevice* device, MetalCommandQueue* queue);
        GraphicsResultRefPtr<GraphicsCommandList> AllocateGraphicsCommandList() override;
        GraphicsResultRefPtr<CopyCommandList> AllocateCopyCommandList() override;
    private:
        MetalCommandQueue* queue = nullptr;
    };
}
