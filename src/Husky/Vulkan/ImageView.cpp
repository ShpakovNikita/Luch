#include <Husky/Vulkan/ImageView.h>
#include <Husky/Vulkan/GraphicsDevice.h>

namespace Husky::Vulkan
{
    ImageView::ImageView(GraphicsDevice* aDevice, vk::ImageView aImageView)
        : device(aDevice)
        , imageView(aImageView)
    {
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
