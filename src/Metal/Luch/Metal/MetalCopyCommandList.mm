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
        , MetalCommandList(aCommandBuffer)
    {
    }

    void MetalCopyCommandList::SetLabel(const String& aLabel)
    {
        label = aLabel;
    }

    void MetalCopyCommandList::Begin()
    {
        commandEncoder = commandBuffer.BlitCommandEncoder();
    }

    void MetalCopyCommandList::End()
    {
        commandBuffer.SetLabel(ns::String{ label.c_str() });
        commandEncoder.SetLabel(ns::String{ label.c_str() });
        commandEncoder.EndEncoding();
        commandEncoder = {};
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

    void MetalCopyCommandList::CopyTextureToTexture(
        Texture* source,
        Texture* destination,
        const TextureToTextureCopy& copy)
    {
        auto mtlSource = static_cast<MetalTexture*>(source)->texture;
        auto mtlDestination = static_cast<MetalTexture*>(destination)->texture;
        commandEncoder.Copy(
            mtlSource,
            copy.sourceSlice,
            copy.sourceLevel,
            { (uint32)copy.sourceOrigin.x, (uint32)copy.sourceOrigin.y, 0 },
            { (uint32)copy.sourceSize.width, (uint32)copy.sourceSize.height, 1 },
            mtlDestination,
            copy.destinationSlice,
            copy.destinationLevel,
            { (uint32)copy.destinationOrigin.x, (uint32)copy.destinationOrigin.y, 0 });
    }

    void MetalCopyCommandList::GenerateMipMaps(Texture* texture)
    {
        auto mtlTexture = static_cast<MetalTexture*>(texture)->texture;

        commandEncoder.GenerateMipmaps(mtlTexture);
    }
}

