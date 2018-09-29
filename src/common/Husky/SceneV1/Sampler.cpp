#include <Husky/SceneV1/Sampler.h>
#include <Husky/Graphics/Sampler.h>

namespace Husky::SceneV1
{
    Sampler::Sampler(
        const Graphics::SamplerCreateInfo& aSamplerDescription,
        const String& aName)
        : name(aName)
        , samplerDescription(aSamplerDescription)
    {
    }
}
