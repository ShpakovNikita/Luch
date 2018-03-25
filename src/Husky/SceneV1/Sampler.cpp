#include <Husky/SceneV1/Sampler.h>
#include <Husky/Vulkan/Sampler.h>

namespace Husky::SceneV1
{
    Sampler::Sampler(const String& aName)
        : name(aName)
    {
    }

    Sampler::~Sampler() = default;
}
