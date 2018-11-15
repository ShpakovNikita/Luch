#include <Luch/SceneV1/Sampler.h>
#include <Luch/Graphics/Sampler.h>

namespace Luch::SceneV1
{
    Sampler::Sampler(
        const Graphics::SamplerCreateInfo& aSamplerDescription,
        const String& aName)
        : name(aName)
        , samplerDescription(aSamplerDescription)
    {
    }
}
