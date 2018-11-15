#include <Luch/Metal/MetalCopyCommandList.h>
#include <Luch/Metal/MetalGraphicsDevice.h>
#include <Luch/Metal/MetalBuffer.h>
#include <Luch/Metal/MetalTexture.h>
#include <Luch/Assert.h>

using namespace Luch::Graphics;

namespace Luch::Metal
{
    MetalCopyCommandList::MetalCopyCommandList(
        MetalGraphicsDevice* device,
        mtlpp::CommandBuffer aCommandBuffer)
        : CopyCommandList(device)
        , commandBuffer(aCommandBuffer)
    {
    }

    void MetalCopyCommandList::Begin()
    {
        commandEncoder = commandBuffer.BlitCommandEncoder();
    }

    void MetalCopyCommandList::End()
    {
        commandEncoder.EndEncoding();
    }

    void MetalCopyCommandList::CopyBufferToTexture(
        Buffer* buffer,
        Texture* texture,
        const BufferToTextureCopy& copy)
    {
        auto mtlBuffer = static_cast<MetalBuffer*>(buffer)->buffer;
        auto mtlTexture = static_cast<MetalTexture*>(texture)->texture;
        commandEncoder.Copy(
            mtlBuffer,
            copy.bufferOffset,
            copy.bytesPerRow,
            copy.bytesPerImage,
            { (uint32)copy.sourceSize.width, (uint32)copy.sourceSize.height, 1 },
            mtlTexture,
            copy.destinationSlice,
            copy.destinationMipmapLevel,
            { (uint32)copy.destinationOrigin.x, (uint32)copy.destinationOrigin.y, 0 });
    }
}

