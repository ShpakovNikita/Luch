#pragma once

#include <Luch/Graphics/CopyCommandList.h>
#include <Luch/Metal/MetalForwards.h>
#include <Luch/Metal/MetalCommandList.h>

namespace Luch::Metal
{
    using namespace Graphics;

    class MetalCopyCommandList 
        : public CopyCommandList
        , public MetalCommandList
    {
        friend class MetalCommandQueue;
    public:
        MetalCopyCommandList(
            MetalGraphicsDevice* device,
            mtlpp::CommandBuffer commandBuffer);

        void SetLabel(const String& label) override;

        void Begin() override;
        void End() override;

        void CopyBufferToTexture(
            Buffer* buffer,
            Texture* texture,
            const BufferToTextureCopy& copy) override;

        void CopyTextureToTexture(
            Texture* source,
            Texture* destination,
            const TextureToTextureCopy& copy) override;

        void GenerateMipMaps(Texture* texture) override;
    private:
        mtlpp::BlitCommandEncoder commandEncoder;
        String label;
    };
}


