#pragma once

#include <Husky/Vulkan.h>
#include <Husky/BaseObject.h>

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

    class ImageViewObject : public BaseObject
    {
    public:
        inline explicit ImageViewObject(ImageView aImageView)
            : imageView(std::move(aImageView))
        {
        }

        inline ImageView* GetImageView() { return &imageView; }
    private:
        ImageView imageView;
    };
}
