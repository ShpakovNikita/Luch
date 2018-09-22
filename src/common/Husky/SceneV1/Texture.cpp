#include <Husky/SceneV1/Texture.h>
#include <Husky/SceneV1/Sampler.h>
#include <Husky/SceneV1/Image.h>

namespace Husky::SceneV1
{
    Texture::Texture(
        const RefPtr<Sampler>& aSampler,
        const RefPtr<Image>& aHostImage,
        const String& aName)
        : name(aName)
        , hostImage(aHostImage)
        , sampler(aSampler)
    {
    }
}
