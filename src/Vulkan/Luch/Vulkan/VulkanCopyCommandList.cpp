#include <Luch/Vulkan/VulkanCopyCommandList.h>

namespace Luch::Vulkan
{
    VulkanCopyCommandList::VulkanCopyCommandList(
        VulkanGraphicsDevice* aDevice,
        vk::CommandBuffer aCommandBuffer)
        : CopyCommandList(aDevice)
        , device(aDevice)
        , commandBuffer(aCommandBuffer)
    {
    }

    void VulkanCopyCommandList::Begin()
    {
        vk::CommandBufferBeginInfo beginInfo;
        commandBuffer.begin(beginInfo);
    }

    void VulkanCopyCommandList::End()
    {
        commandBuffer.end();
    }

    void VulkanCopyCommandList::CopyBufferToTexture(
            Buffer* buffer, Texture* texture, const BufferToTextureCopy& copy)
    {
        // todo: implement
        /*
        VulkanDeviceBuffer* buffer,
        VulkanImage* image,
        vk::ImageLayout dstLayout
        */
        /*
         *
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
         */
        /*
        vk::ImageSubresourceLayers subresource;
        subresource.setAspectMask(ToVulkanImageAspectFlags(image->GetImageAspects()));
        subresource.setBaseArrayLayer(0);
        subresource.setLayerCount(1);
        subresource.setMipLevel(0);

        vk::BufferImageCopy bufferImageCopy;
        bufferImageCopy.setBufferOffset(copy.bufferOffset);
        bufferImageCopy.setBufferRowLength(copy.bufferRowLength);
        bufferImageCopy.setBufferImageHeight(copy.bufferImageHeight);
        bufferImageCopy.setImageOffset(copy.imageOffset);
        bufferImageCopy.setImageExtent(copy.imageExtent);
        bufferImageCopy.setImageSubresource(subresource);

        commandBuffer.copyBufferToImage(
            buffer->GetBuffer(),
            image->GetImage(),
            dstLayout,
            { bufferImageCopy }
        );
        */
    }
}
