#pragma once

#include <Luch/BaseObject.h>
#include <Luch/RefPtr.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/SceneV1/SceneV1Forwards.h>
#include <Luch/SceneV1/Sampler.h>

namespace Luch::SceneV1
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
