#pragma once

#include <Husky/BaseObject.h>
#include <Husky/RefPtr.h>
#include <Husky/Vulkan/Forwards.h>
#include <Husky/Vulkan.h>

namespace Husky::SceneV1
{
    class Sampler : public BaseObject
    {
    public:
        Sampler(
            const vk::SamplerCreateInfo& samplerDescription, // TODO
            const String& name = "");

        ~Sampler();
    private:
        String name;

        vk::SamplerCreateInfo samplerDescription;

        RefPtr<Vulkan::Sampler> sampler;
    };
}
