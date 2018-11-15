#pragma once

#include <Luch/Graphics/Texture.h>
#include <Luch/Graphics/TextureCreateInfo.h>
#include <Luch/Metal/MetalForwards.h>

namespace Luch::Metal
{
    using namespace Graphics;

    class MetalTexture : public Texture
    {
        friend class MetalSwapchain;
        friend class MetalDescriptorSet;
        friend class MetalCopyCommandList;
    public:
        MetalTexture(
            MetalGraphicsDevice* device,
            const TextureCreateInfo& createInfo,
            mtlpp::Texture texture);

        const TextureCreateInfo& GetCreateInfo() const override { return createInfo; }
        mtlpp::Texture GetNativeTexture() { return texture; }
    private:
        TextureCreateInfo createInfo;
        mtlpp::Texture texture;
    };
}
