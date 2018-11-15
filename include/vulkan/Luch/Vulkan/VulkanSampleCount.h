#pragma once

#include <Luch/Types.h>
#include <Luch/Vulkan.h>
#include <Luch/SampleCount.h>

namespace Luch::Vulkan
{
    // TODO optimize
    inline vk::SampleCountFlagBits ToVulkanSampleCount(SampleCount sampleCount)
    {
        switch (sampleCount)
        {
        case SampleCount::e1:
            return vk::SampleCountFlagBits::e1;
        case SampleCount::e2:
            return vk::SampleCountFlagBits::e2;
        case SampleCount::e4:
            return vk::SampleCountFlagBits::e4;
        case SampleCount::e8:
            return vk::SampleCountFlagBits::e8;
        case SampleCount::e16:
            return vk::SampleCountFlagBits::e16;
        case SampleCount::e32:
            return vk::SampleCountFlagBits::e32;
        case SampleCount::e64:
            return vk::SampleCountFlagBits::e64;
        default:
            LUCH_ASSERT_MSG(false, "Unknown sample count");
        }
    }
}
