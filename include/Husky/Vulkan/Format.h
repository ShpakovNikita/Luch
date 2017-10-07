#pragma once

#include <Husky/Vulkan.h>
#include <Husky/Format.h>

namespace Husky::Vulkan
{
    vk::Format ToVulkanFormat(Format format);
    Format FromVulkanFormat(vk::Format format);
}
