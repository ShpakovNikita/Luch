#include <Husky/SceneV1/Texture.h>
#include <Husky/SceneV1/Sampler.h>
#include <Husky/Vulkan/Image.h>
#include <Husky/Vulkan/ImageView.h>

namespace Husky::SceneV1
{
    Texture::Texture(const RefPtr<Sampler>& aSampler, const String& aName)
        : sampler(aSampler)
        , name(aName)
    {
    }

    Texture::~Texture() = default;
}