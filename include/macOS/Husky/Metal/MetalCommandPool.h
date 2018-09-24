#pragma once

#include <Husky/Graphics/CommandPool.h>
#include <Husky/Metal/MetalForwards.h>

namespace Husky::Metal
{
    using namespace Graphics;

    class MetalCommandPool : public CommandPool
    {
    public:
        MetalCommandPool(MetalGraphicsDevice* device, MetalCommandQueue* queue);
        GraphicsResultRefPtr<GraphicsCommandList> AllocateGraphicsCommandList() override;
    private:
        MetalCommandQueue* queue = nullptr;
    };
}
