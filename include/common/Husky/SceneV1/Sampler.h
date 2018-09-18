#pragma once

#include <Husky/BaseObject.h>
#include <Husky/RefPtr.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Forwards.h>

namespace Husky::SceneV1
{
    class VulkanSampler : public BaseObject
    {
    public:
        VulkanSampler(
            const vk::SamplerCreateInfo& samplerDescription, // TODO
            const String& name = "");

        inline const vk::SamplerCreateInfo& GetSamplerDescription() const { return samplerDescription; }
        inline const RefPtr<Vulkan::Sampler>& GetDeviceSampler() const { return sampler; }
        inline void SetDeviceSampler(const RefPtr<Vulkan::Sampler>& aSampler) { sampler = aSampler; }
    private:
        String name;

        vk::SamplerCreateInfo samplerDescription;

        RefPtr<Vulkan::Sampler> sampler;
    };
}
