#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Forwards.h>

namespace Husky::Vulkan
{
    class ImageView : public BaseObject
    {
        friend class GraphicsDevice;
    public:
        ImageView(GraphicsDevice* device, vk::ImageView imageView);
        ~ImageView();

        inline GraphicsDevice* GetDevice() { return device; }
        inline vk::ImageView GetImageView() { return imageView; }
    private:
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::ImageView imageView;
    };
}
