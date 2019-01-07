#pragma once

#include <Luch/Assert.h>
#include <Luch/Vulkan.h>
#include <Luch/Graphics/IndexType.h>

namespace Luch::Vulkan
{
    inline vk::IndexType ToVulkanIndexType(IndexType indexType)
    {
        switch (indexType)
        {
        case IndexType::UInt16:
            return vk::IndexType::eUint16;
        case IndexType::UInt32:
            return vk::IndexType::eUint32;
        default:
            LUCH_ASSERT_MSG(false, "Unknown index type");
            return vk::IndexType::eUint16;
        }
    }

    inline IndexType FromVulkanIndexType(vk::IndexType indexType)
    {
        switch (indexType)
        {
        case vk::IndexType::eUint16:
            return IndexType::UInt16;
        case vk::IndexType::eUint32:
            return IndexType::UInt32;
        default:
            LUCH_ASSERT_MSG(false, "Unknown index type");
            return IndexType::UInt16;
        }
    }
}
