#include <Husky/SceneV1/Texture.h>
#include <Husky/SceneV1/Sampler.h>
#include <Husky/SceneV1/Image.h>
#include <Husky/Vulkan/GraphicsDevice.h>
#include <Husky/Vulkan/Image.h>
#include <Husky/Vulkan/ImageView.h>
#include <Husky/Vulkan/Buffer.h>
#include <Husky/Vulkan/CommandPool.h>

namespace Husky::SceneV1
{
    Texture::Texture(
        const RefPtr<Sampler>& aSampler,
        const RefPtr<Image>& aHostImage,
        const String& aName)
        : sampler(aSampler)
        , hostImage(aHostImage)
        , name(aName)
    {
    }
}
