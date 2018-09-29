#include <Husky/Metal/MetalCopyCommandList.h>
#include <Husky/Metal/MetalGraphicsDevice.h>
#include <Husky/Metal/MetalBuffer.h>
#include <Husky/Metal/MetalTexture.h>
#include <Husky/Assert.h>

using namespace Husky::Graphics;

namespace Husky::Metal
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

