#include <Husky/Metal/MetalTexture.h>
#include <Husky/Metal/MetalGraphicsDevice.h>

namespace Husky::Metal
{
    using namespace Graphics;

    MetalTexture::MetalTexture(
        MetalGraphicsDevice* device,
        const TextureCreateInfo& aCreateInfo,
        mtlpp::Texture aTexture)
        : Texture(device)
        , createInfo(aCreateInfo)
        , texture(aTexture)
    {
    }
}
