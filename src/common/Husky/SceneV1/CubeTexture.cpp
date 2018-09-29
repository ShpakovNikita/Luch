#include <Husky/SceneV1/CubeTexture.h>
#include <Husky/SceneV1/Sampler.h>
#include <Husky/SceneV1/Image.h>
#include <Husky/Graphics/Texture.h>

namespace Husky::SceneV1
{
    CubeTexture::CubeTexture(
        const RefPtr<Sampler>& aSampler,
        const String& aName)
        : name(aName)
        , sampler(aSampler)
    {
    }
}
