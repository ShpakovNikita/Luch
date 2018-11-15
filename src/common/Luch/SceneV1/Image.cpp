#include <Luch/SceneV1/Image.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Luch::SceneV1
{
    RefPtr<Image> Image::LoadFromFile(const String& filename)
    {
        int32 width;
        int32 height;
        int32 realComponentCount;
        int32 requiredComponentCount = 4;

        // Force 4 components for now
        const uchar8* data = stbi_load(filename.c_str(), &width, &height, &realComponentCount, requiredComponentCount);
        if (data)
        {
            int32 sizeInBytes = width * height * requiredComponentCount;
            Vector<Byte> imageBuffer;
            imageBuffer.resize(sizeInBytes);
            memcpy(imageBuffer.data(), data, sizeInBytes);

            Graphics::Format format = Graphics::Format::Undefined;

            switch (requiredComponentCount)
            {
            case 1:
                format = Graphics::Format::R8Unorm;
                break;
            case 2:
                format = Graphics::Format::R8G8Unorm;
                break;
            case 3:
                format = Graphics::Format::R8G8B8Unorm;
                break;
            case 4:
                format = Graphics::Format::R8G8B8A8Unorm;
                break;
            default:
                LUCH_ASSERT(false);
            }

            auto image = MakeRef<Image>();
            image->SetWidth(width);
            image->SetHeight(height);
            image->SetBuffer(std::move(imageBuffer));
            image->SetBytesPerPixel(requiredComponentCount); // 8 bites per channel (component)
            image->SetFormat(format);

            return image;
        }
        else
        {
            return nullptr;
        }
    }
}
