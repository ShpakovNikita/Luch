#pragma once

#include <Husky/Graphics/Texture.h>
#include <Husky/Graphics/TextureCreateInfo.h>
#include <Husky/Metal/MetalForwards.h>

namespace Husky::Metal
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
