#pragma once

#include <Luch/ArrayProxy.h>
#include <Luch/BaseObject.h>
#include <Luch/Graphics/GraphicsObject.h>
#include <Luch/Graphics/Point2D.h>
#include <Luch/Graphics/Size2D.h>

namespace Luch::Graphics
{
    struct BufferToTextureCopy
    {
        int32 bufferOffset = 0;
        int32 bytesPerRow = 0;
        int32 bytesPerImage = 0;
        IntSize2D sourceSize;

        int32 destinationSlice = 0;
        int32 destinationMipmapLevel = 0;
        IntPoint2D destinationOrigin;
    };

    class CopyCommandList : public GraphicsObject
    {
    public:
        CopyCommandList(GraphicsDevice* device) : GraphicsObject(device) {}

        virtual void Begin() = 0;
        virtual void End() = 0;

        virtual void CopyBufferToTexture(Buffer* buffer, Texture* texture, const BufferToTextureCopy& copy) = 0;
    };
}
