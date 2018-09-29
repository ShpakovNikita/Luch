#pragma once

#include <Husky/BaseObject.h>
#include <Husky/RefPtr.h>
#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/Graphics/SamplerCreateInfo.h>

namespace Husky::SceneV1
{
    class Sampler : public BaseObject
    {
    public:
        Sampler(
            const Graphics::SamplerCreateInfo& samplerDescription,
            const String& name = "");

        inline const Graphics::SamplerCreateInfo& GetSamplerDescription() const { return samplerDescription; }
        inline const RefPtr<Graphics::Sampler>& GetDeviceSampler() const { return sampler; }
        inline void SetDeviceSampler(const RefPtr<Graphics::Sampler>& aSampler) { sampler = aSampler; }
    private:
        String name;

        Graphics::SamplerCreateInfo samplerDescription;
        RefPtr<Graphics::Sampler> sampler;
    };
}
