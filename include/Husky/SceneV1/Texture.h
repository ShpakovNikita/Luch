#pragma once

#include <Husky/BaseObject.h>
#include <Husky/RefPtr.h>
#include <Husky/Vulkan/Forwards.h>
#include <Husky/SceneV1/Forwards.h>

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

        ~Texture();
        // void LoadToHost()
        // void LoadToDevice()

        inline const RefPtr<Sampler>& GetSampler() const { return sampler; }
    private:
        String name;

        RefPtr<Image> hostImage;
        RefPtr<Vulkan::Image> deviceImage;
        RefPtr<Vulkan::ImageView> deviceImageView;

        RefPtr<Sampler> sampler;
    };
}