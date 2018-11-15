#include <Luch/SceneV1/Texture.h>
#include <Luch/SceneV1/Sampler.h>
#include <Luch/SceneV1/Image.h>

namespace Luch::SceneV1
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
