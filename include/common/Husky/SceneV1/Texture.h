#pragma once

#include <Husky/BaseObject.h>
#include <Husky/RefPtr.h>
#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/SceneV1/SceneV1Forwards.h>
#include <Husky/SceneV1/Sampler.h>

namespace Husky::SceneV1
{
    struct TextureSource
    {
        String root;
        String filename;
        // TODO
    };

    class Texture : public BaseObject
    {
    public:
        Texture(
            const RefPtr<Sampler>& sampler,
            const RefPtr<Image>& hostImage,
            const String& name = "");

        inline const RefPtr<Image>& GetHostImage() const { return hostImage; }
        inline const RefPtr<Graphics::Texture>& GetDeviceTexture() const { return deviceTexture; }

        inline void SetDeviceTexture(const RefPtr<Graphics::Texture>& aDeviceTexture) { deviceTexture = aDeviceTexture; }

        inline const RefPtr<Sampler>& GetSampler() const { return sampler; }
        inline const RefPtr<Graphics::Sampler>& GetDeviceSampler() const { return sampler->GetDeviceSampler(); }

        inline bool IsSRGB() const { return isSRGB; }
        inline void SetSRGB(bool srgb) { isSRGB = srgb; }
    private:
        String name;

        RefPtr<Image> hostImage;
        RefPtr<Graphics::Texture> deviceTexture;
        bool isSRGB = false;

        RefPtr<Sampler> sampler;
    };
}
