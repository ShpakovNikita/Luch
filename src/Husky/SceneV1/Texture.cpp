#pragma once

#include <Husky/BaseObject.h>
#include <Husky/RefPtr.h>
#include <Husky/Vulkan/Forwards.h>
#include <Husky/SceneV1/Forwards.h>

namespace Husky::SceneV1
{
    class Texture : public BaseObject
    {
    public:
        Texture(
            const RefPtr<Sampler>& sampler,
            const RefPtr<Vulkan::ImageObject>& image,
            const RefPtr<Vulkan::ImageViewObject>& imageViewObject,
            const String& name = "");
    private:
        String name;
        RefPtr<Vulkan::ImageObject> image;
        RefPtr<Vulkan::ImageViewObject> imageView;
        RefPtr<Sampler> sampler;
    };
}