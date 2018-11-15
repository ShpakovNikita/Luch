#include <Luch/Metal/MetalTexture.h>
#include <Luch/Metal/MetalGraphicsDevice.h>

namespace Luch::Metal
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
