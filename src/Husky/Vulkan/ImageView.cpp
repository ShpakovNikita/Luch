#include <Husky/Vulkan/ImageView.h>
#include <Husky/Vulkan/GraphicsDevice.h>

namespace Husky::Vulkan
{
    ImageView::ImageView(GraphicsDevice* aDevice, vk::ImageView aImageView)
        : device(aDevice)
        , imageView(aImageView)
    {
    }

    ImageView::ImageView(ImageView&& other)
        : device(other.device)
        , imageView(other.imageView)
    {
    }

    ImageView& ImageView::operator=(ImageView&& other)
    {
        Destroy();

        device = other.device;
        imageView = other.imageView;

        other.device = nullptr;
        other.imageView = nullptr;

        return *this;
    }

    ImageView::~ImageView()
    {
        Destroy();
    }

    void ImageView::Destroy()
    {
        if (device)
        {
            device->DestroyImageView(this);
        }
    }
}
