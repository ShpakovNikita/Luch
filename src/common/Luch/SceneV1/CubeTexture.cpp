#include <Luch/SceneV1/CubeTexture.h>
#include <Luch/SceneV1/Sampler.h>
#include <Luch/SceneV1/Image.h>
#include <Luch/Graphics/Texture.h>

namespace Luch::SceneV1
{
    CubeTexture::CubeTexture(
        const RefPtr<Sampler>& aSampler,
        const String& aName)
        : name(aName)
        , sampler(aSampler)
    {
    }
}
