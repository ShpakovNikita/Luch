#pragma once

#include <Husky/BaseObject.h>
#include <Husky/RefPtr.h>
#include <Husky/Vulkan/Forwards.h>

namespace Husky::SceneV1
{
    class Sampler : public BaseObject
    {
    public:
        Sampler(
            const RefPtr<Vulkan::SamplerObject>& sampler,
            const String& name = "");
    private:
        String name;
        RefPtr<Vulkan::SamplerObject> sampler;
    };
}
