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
    public:
        MetalTexture(
            MetalGraphicsDevice* device,
            const TextureCreateInfo& createInfo,
            mtlpp::Texture texture);

        const TextureCreateInfo& GetCreateInfo() const override { return createInfo; }
    private:
        TextureCreateInfo createInfo;
        mtlpp::Texture texture;
    };
}
