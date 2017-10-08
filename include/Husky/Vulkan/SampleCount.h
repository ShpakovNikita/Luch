#pragma once

#include <Husky/Types.h>
#include <Husky/Vulkan.h>
#include <Husky/SampleCount.h>

namespace Husky::Vulkan
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
            HUSKY_ASSERT(false, "Unknown sample count");
        }
    }
}
