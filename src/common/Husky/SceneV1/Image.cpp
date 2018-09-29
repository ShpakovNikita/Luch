#include <Husky/SceneV1/Image.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Husky::SceneV1
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
            Vector<uint8> imageBuffer;
            imageBuffer.resize(sizeInBytes);
            memcpy(imageBuffer.data(), data, sizeInBytes);

            return MakeRef<Image>(width, height, 4, move(imageBuffer));
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
        : width(aWidth)
        , height(aHeight)
        , componentCount(aComponentCount)
        , buffer(move(aBuffer))
    {
    }
}