#pragma once

#include <Luch/BaseObject.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/Common/VulkanForwards.h>
#include <Luch/Vulkan/Queue/VulkanQueueInfo.h>

namespace Luch::Vulkan
{
    class BufferMemoryBarrier
    {
        friend struct VulkanPipelineBarrier;
    public:
        static vk::BufferMemoryBarrier ToVulkanBufferMemoryBarrier(const BufferMemoryBarrier& bufferMemoryBarrier);

        inline BufferMemoryBarrier& FromAccess(vk::AccessFlags aSrcAccess)
        {
            srcAccess = aSrcAccess;
            return *this;
        }

        inline BufferMemoryBarrier& ToAccess(vk::AccessFlags aDstAcces)
        {
            dstAccess = aDstAcces;
            return *this;
        }

        inline BufferMemoryBarrier& FromQueue(QueueIndex aSrcQueue)
        {
            srcQueue = aSrcQueue;
            return *this;
        }

        inline BufferMemoryBarrier& ToQueue(QueueIndex aDstQueue)
        {
            dstQueue = aDstQueue;
            return *this;
        }

        inline BufferMemoryBarrier& ForBuffer(VulkanDeviceBuffer* aBuffer, int32 aSize, int32 aOffset)
        {
            buffer = aBuffer;
            size = aSize;
            offset = aOffset;
            return *this;
        }
    private:
        VulkanDeviceBuffer* buffer;
        int32 size = 0;
        int32 offset = 0;
        vk::AccessFlags srcAccess;
        vk::AccessFlags dstAccess;
        QueueIndex srcQueue;
        QueueIndex dstQueue;
        bool byRegion = false;
    };

    class ImageMemoryBarrier
    {
        friend struct VulkanPipelineBarrier;
    public:
        static vk::ImageMemoryBarrier ToVulkanImageMemoryBarrier(const ImageMemoryBarrier& imageMemoryBarrier);

        inline ImageMemoryBarrier& FromAccess(vk::AccessFlags aSrcAccess)
        {
            srcAccess = aSrcAccess;
            return *this;
        }

        inline ImageMemoryBarrier& ToAccess(vk::AccessFlags aDstAcces)
        {
            dstAccess = aDstAcces;
            return *this;
        }

        inline ImageMemoryBarrier& FromLayout(vk::ImageLayout aOldLayout)
        {
            oldLayout = aOldLayout;
            return *this;
        }

        inline ImageMemoryBarrier& ToLayout(vk::ImageLayout aNewLayout)
        {
            newLayout = aNewLayout;
            return *this;
        }

        inline ImageMemoryBarrier& FromQueue(QueueIndex aSrcQueue)
        {
            srcQueue = aSrcQueue;
            return *this;
        }

        inline ImageMemoryBarrier& ToQueue(QueueIndex aDstQueue)
        {
            dstQueue = aDstQueue;
            return *this;
        }

        inline ImageMemoryBarrier& ForImage(VulkanImage* aImage)
        {
            image = aImage;
            return *this;
        }
    private:
        VulkanImage* image;
        vk::AccessFlags srcAccess;
        vk::AccessFlags dstAccess;
        QueueIndex srcQueue;
        QueueIndex dstQueue;
        vk::ImageLayout oldLayout;
        vk::ImageLayout newLayout;
        bool byRegion = false;
    };

    struct VulkanPipelineBarrier
    {
        vk::PipelineStageFlags srcStageMask;
        vk::PipelineStageFlags dstStageMask;
        vk::DependencyFlags dependencyFlags;
        Vector<vk::BufferMemoryBarrier> bufferMemoryBarriers;
        Vector<vk::ImageMemoryBarrier> imageMemoryBarriers;
    };

    class PipelineBarrier
    {
    public:
        static VulkanPipelineBarrier ToVulkanPipelineBarrier(const PipelineBarrier& barrier);

        inline PipelineBarrier& FromStage(vk::PipelineStageFlags aSrcStage)
        {
            srcStage = aSrcStage;
            return *this;
        }

        inline PipelineBarrier& ToStage(vk::PipelineStageFlags aDstStage)
        {
            dstStage = aDstStage;
            return *this;
        }

        inline PipelineBarrier& ByRegion(bool aByRegion = true)
        {
            byRegion = aByRegion;
            return *this;
        }

        inline PipelineBarrier& WithBufferBarrier(const BufferMemoryBarrier* barrier)
        {
            bufferMemoryBarriers.push_back(barrier);
            return *this;
        }

        inline PipelineBarrier& WithImageBarrier(const ImageMemoryBarrier* barrier)
        {
            imageMemoryBarriers.push_back(barrier);
            return *this;
        }
    private:
        vk::PipelineStageFlags srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
        vk::PipelineStageFlags dstStage = vk::PipelineStageFlagBits::eBottomOfPipe;
        Vector<const BufferMemoryBarrier*> bufferMemoryBarriers;
        Vector<const ImageMemoryBarrier*> imageMemoryBarriers;
        bool byRegion = false;
    };
}
