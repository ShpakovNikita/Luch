#pragma once

#include <Husky/Graphics/CopyCommandList.h>
#include <Husky/Metal/MetalForwards.h>

namespace Husky::Metal
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


