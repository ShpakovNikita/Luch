#pragma once

#include <Husky/BaseObject.h>
#include <Husky/RefPtr.h>
#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/SceneV1/SceneV1Forwards.h>
#include <Husky/SceneV1/Sampler.h>

namespace Husky::SceneV1
{
    class CubeTexture : public BaseObject
    {
    public:
        CubeTexture(
            const RefPtr<Sampler>& sampler,
            const String& name = "");

        inline const RefPtr<Graphics::Texture>& GetDeviceTexture() const { return deviceTexture; }

        inline void SetDeviceTexture(const RefPtr<Graphics::Texture>& aTexture) { deviceTexture = aTexture; }


        inline const RefPtr<Sampler>& GetSampler() const { return sampler; }
        inline const RefPtr<Graphics::Sampler>& GetDeviceSampler() const { return sampler->GetDeviceSampler(); }
    private:
        String name;

        RefPtr<Graphics::Texture> deviceTexture;

        RefPtr<Sampler> sampler;
    };
}
