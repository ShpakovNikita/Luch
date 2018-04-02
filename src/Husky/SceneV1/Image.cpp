#include <Husky/SceneV1/Image.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Husky::SceneV1
{
    RefPtr<Image> Image::LoadFromFile(const String& filename)
    {
        int32 width;
        int32 height;
        int32 componentCount;

        const uchar8* data = stbi_load(filename.c_str(), &width, &height, &componentCount, 0);
        if (data)
        {
            int32 sizeInBytes = width * height * componentCount;
            Vector<uint8> imageBuffer;
            imageBuffer.resize(sizeInBytes);
            memcpy(imageBuffer.data(), data, sizeInBytes);

            return MakeRef<Image>(width, height, componentCount, move(imageBuffer));
        }
        else
        {
            return nullptr;
        }
    }

    Image::Image(
        int32 aWidth,
        int32 aHeight,
        int32 aComponentCount,
        Vector<uint8>&& aBuffer)
        : width(width)
        , height(height)
        , componentCount(aComponentCount)
        , buffer(move(aBuffer))
    {
    }
}