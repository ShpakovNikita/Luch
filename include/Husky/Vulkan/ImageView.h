#pragma once

#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;

    class ImageView
    {
        friend class GraphicsDevice;
    public:
        ImageView() = default;

        ImageView(ImageView&& other);
        ImageView& operator=(ImageView&& other);

        ~ImageView();

        inline vk::ImageView GetImageView() { return imageView; }
    private:
        ImageView(GraphicsDevice* device, vk::ImageView imageView);
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::ImageView imageView;
    };
}
