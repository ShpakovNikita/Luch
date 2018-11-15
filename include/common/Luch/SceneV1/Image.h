#pragma once

#include <Luch/BaseObject.h>
#include <Luch/RefPtr.h>
#include <Luch/Graphics/Format.h>

namespace Luch::SceneV1
{
    class Image : public BaseObject
    {
    public:
        static RefPtr<Image> LoadFromFile(const String& filename);

        Image() = default;
        ~Image() = default;

        inline int32 GetWidth() const { return width; }
        inline void SetWidth(int32 aWidth) { width = aWidth; }

        inline int32 GetHeight() const { return height; }
        inline void SetHeight(int32 aHeight) { height = aHeight; }

        inline const Vector<Byte>& GetBuffer() const { return  buffer; }
        inline void SetBuffer(Vector<Byte> aBuffer) { buffer = std::move(aBuffer); }

        inline int32 GetBytesPerPixel() const { return bytesPerPixel; }
        inline void SetBytesPerPixel(int32 bpp) { bytesPerPixel = bpp; }

        inline Graphics::Format GetFormat() const { return format; }
        inline void SetFormat(Graphics::Format aFormat) { format = aFormat; }
    private:
        int32 width = 0;
        int32 height = 0;
        int32 bytesPerPixel = 0;
        Graphics::Format format = Graphics::Format::Undefined;

        Vector<Byte> buffer;
    };
}
