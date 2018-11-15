#pragma once

#include <Luch/Graphics/CopyCommandList.h>
#include <Luch/Metal/MetalForwards.h>

namespace Luch::Metal
{
    using namespace Graphics;

    class MetalCopyCommandList : public CopyCommandList
    {
        friend class MetalCommandQueue;
    public:
        MetalCopyCommandList(
            MetalGraphicsDevice* device,
            mtlpp::CommandBuffer commandBuffer);

        void Begin() override;
        void End() override;

        void CopyBufferToTexture(
            Buffer* buffer,
            Texture* texture,
            const BufferToTextureCopy& copy) override;
    private:
        mtlpp::CommandBuffer commandBuffer;
        mtlpp::BlitCommandEncoder commandEncoder;
    };
}


