#pragma once

#include <Luch/BaseObject.h>
#include <Luch/RefPtr.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/SamplerCreateInfo.h>

namespace Luch::SceneV1
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
