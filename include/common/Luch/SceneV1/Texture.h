#pragma once

#include <Luch/BaseObject.h>
#include <Luch/RefPtr.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/SceneV1/SceneV1Forwards.h>
#include <Luch/SceneV1/Sampler.h>

namespace Luch::SceneV1
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
        Texture() = default;

        inline const RefPtr<Image>& GetHostImage() const { return hostImage; }
        inline void SetHostImage(const RefPtr<Image>& image) { hostImage = image; }

        inline const RefPtr<Graphics::Texture>& GetDeviceTexture() const { return deviceTexture; }
        inline void SetDeviceTexture(const RefPtr<Graphics::Texture>& aDeviceTexture) { deviceTexture = aDeviceTexture; }

        const String& GetName() const { return name; }
        void SetName(const String& aName) { name = aName; }

        inline const RefPtr<Sampler>& GetSampler() const { return sampler; }
        inline void SetSampler(const RefPtr<Sampler>& aSampler) { sampler = aSampler; }

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
