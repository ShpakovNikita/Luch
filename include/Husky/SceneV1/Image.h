#pragma once

#include <Husky/BaseObject.h>
#include <Husky/RefPtr.h>

namespace Husky::SceneV1
{
    class Image : public BaseObject
    {
    public:
        static RefPtr<Image> LoadFromFile(const String& filename);

        Image(
            int32 width,
            int32 height,
            int32 componentCount,
            Vector<uint8>&& buffer);

        ~Image() = default;

        int32 GetWidth() const { return width; }
        int32 GetHeight() const { return height; }
        int32 GetComponentCount() const { return componentCount; }

        const Vector<uint8>& GetBuffer() const { return  buffer; }
    private:
        int32 width;
        int32 height;
        int32 componentCount;

        Vector<uint8> buffer;
    };
}
