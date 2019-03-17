#include <Luch/Vulkan/Pipeline/VulkanPipelineBarrier.h>
#include <Luch/Vulkan/Image/VulkanImage.h>
#include <Luch/Vulkan/Common/VulkanImageAspects.h>
#include <Luch/Vulkan/Buffer/VulkanDeviceBuffer.h>

namespace Luch::Vulkan
{
    vk::BufferMemoryBarrier BufferMemoryBarrier::ToVulkanBufferMemoryBarrier(const BufferMemoryBarrier& bufferMemoryBarrier)
    {
        vk::BufferMemoryBarrier result;
        result.setSrcAccessMask(bufferMemoryBarrier.srcAccess);
        result.setDstAccessMask(bufferMemoryBarrier.dstAccess);
        result.setSrcQueueFamilyIndex(bufferMemoryBarrier.srcQueue);
        result.setDstQueueFamilyIndex(bufferMemoryBarrier.dstQueue);
        result.setSize(bufferMemoryBarrier.size);
        result.setOffset(bufferMemoryBarrier.offset);
        result.setBuffer(bufferMemoryBarrier.buffer->GetBuffer());
        return result;
    }

    vk::ImageMemoryBarrier ImageMemoryBarrier::ToVulkanImageMemoryBarrier(const ImageMemoryBarrier& imageMemoryBarrier)
    {
        vk::ImageAspectFlags aspects = ToVulkanImageAspectFlags(imageMemoryBarrier.image->GetImageAspects());

        vk::ImageSubresourceRange subresourceRange;
        subresourceRange.setBaseArrayLayer(0);
        subresourceRange.setLayerCount(1);
        subresourceRange.setBaseMipLevel(0);
        subresourceRange.setLevelCount(1);
        subresourceRange.setAspectMask(aspects);

        vk::ImageMemoryBarrier result;
        result.setSrcAccessMask(imageMemoryBarrier.srcAccess);
        result.setDstAccessMask(imageMemoryBarrier.dstAccess);
        result.setSrcQueueFamilyIndex(imageMemoryBarrier.srcQueue);
        result.setDstQueueFamilyIndex(imageMemoryBarrier.dstQueue);
        result.setImage(imageMemoryBarrier.image->GetImage());
        result.setSubresourceRange(subresourceRange);
        result.setOldLayout(imageMemoryBarrier.oldLayout);
        result.setNewLayout(imageMemoryBarrier.newLayout);

        return result;
    }

    VulkanPipelineBarrier PipelineBarrier::ToVulkanPipelineBarrier(const PipelineBarrier& barrier)
    {
        VulkanPipelineBarrier pipelineBarrier;

        pipelineBarrier.srcStageMask = barrier.srcStage;
        pipelineBarrier.dstStageMask = barrier.dstStage;

        pipelineBarrier.bufferMemoryBarriers.reserve(barrier.bufferMemoryBarriers.size());

        for (const BufferMemoryBarrier* bufferBarrier : barrier.bufferMemoryBarriers)
        {
            pipelineBarrier.bufferMemoryBarriers.push_back(BufferMemoryBarrier::ToVulkanBufferMemoryBarrier(*bufferBarrier));
        }

        for (const ImageMemoryBarrier* imageBarrier : barrier.imageMemoryBarriers)
        {
            pipelineBarrier.imageMemoryBarriers.push_back(ImageMemoryBarrier::ToVulkanImageMemoryBarrier(*imageBarrier));
        }

        if (barrier.byRegion)
        {
            pipelineBarrier.dependencyFlags |= vk::DependencyFlagBits::eByRegion;
        }

        return pipelineBarrier;
    }

}
