#pragma once

#include <Husky/BaseObject.h>
#include <Husky/RefPtr.h>
#include <Husky/Vulkan/Forwards.h>
#include <Husky/Vulkan/CommandBuffer.h>
#include <Husky/SceneV1/Forwards.h>
#include <Husky/SceneV1/Sampler.h>

namespace Husky::SceneV1
{
    class CubeTexture : public BaseObject
    {
    public:
        CubeTexture(
            const RefPtr<Sampler>& sampler,
            const String& name = "");

        inline const RefPtr<Vulkan::Image>& GetDeviceImage() const { return deviceImage; }
        inline const RefPtr<Vulkan::ImageView>& GetDeviceImageView() const { return deviceImageView; }

        inline void SetDeviceImage(const RefPtr<Vulkan::Image>& aDeviceImage) { deviceImage = aDeviceImage; }
        inline void SetDeviceImageView(const RefPtr<Vulkan::ImageView>& aDeviceImageView) { deviceImageView = aDeviceImageView; }

        inline const RefPtr<Sampler>& GetSampler() const { return sampler; }
        inline const RefPtr<Vulkan::Sampler>& GetDeviceSampler() const { return sampler->GetDeviceSampler(); }
    private:
        String name;

        RefPtr<Vulkan::Image> deviceImage;
        RefPtr<Vulkan::ImageView> deviceImageView;

        RefPtr<Sampler> sampler;
    };
}