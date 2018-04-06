#include <Husky/SceneV1/Sampler.h>
#include <Husky/Vulkan/Sampler.h>

namespace Husky::SceneV1
{
    Sampler::Sampler(
        const vk::SamplerCreateInfo& aSamplerDescription,
        const String& aName)
        : samplerDescription(aSamplerDescription)
        , name(aName)
    {
    }
}
