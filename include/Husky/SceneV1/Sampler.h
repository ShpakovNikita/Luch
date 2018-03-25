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
            const String& name = "");

        ~Sampler();
    private:
        String name;

        //vk::SamplerCreateInfo samplerDescription;

        RefPtr<Vulkan::SamplerObject> sampler;
    };
}
