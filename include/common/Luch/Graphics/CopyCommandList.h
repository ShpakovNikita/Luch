#pragma once

#include <Luch/ArrayProxy.h>
#include <Luch/BaseObject.h>
#include <Luch/Graphics/CommandList.h>
#include <Luch/Graphics/Point2.h>
#include <Luch/Graphics/Size2.h>

namespace Luch::Graphics
{
    struct BufferToTextureCopy
    {
        int32 bufferOffset = 0;
        int32 bytesPerRow = 0;
        int32 bytesPerImage = 0;
        Size2i sourceSize;

        int32 destinationSlice = 0;
        int32 destinationMipmapLevel = 0;
        Point2i destinationOrigin;
    };

    class CopyCommandList : public CommandList
    {
    public:
        CopyCommandList(GraphicsDevice* device) : CommandList(device) {}

        virtual void Begin() = 0;
        virtual void End() = 0;

        CommandListType GetType() const override { return CommandListType::Copy; }

        virtual void CopyBufferToTexture(Buffer* buffer, Texture* texture, const BufferToTextureCopy& copy) = 0;
        virtual void GenerateMipMaps(Texture* texture) = 0;
    };
}
