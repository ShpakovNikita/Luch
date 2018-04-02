#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;

    class ImageView : public BaseObject
    {
        friend class GraphicsDevice;
    public:
        ImageView(GraphicsDevice* device, vk::ImageView imageView);

        ImageView() = default;

        ImageView(ImageView&& other) = delete;
        ImageView(const ImageView& other) = delete;
        ImageView& operator=(const ImageView& other) = delete;
        ImageView& operator=(ImageView&& other) = delete;

        ~ImageView();

        inline vk::ImageView GetImageView() { return imageView; }
    private:
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::ImageView imageView;
    };
}
