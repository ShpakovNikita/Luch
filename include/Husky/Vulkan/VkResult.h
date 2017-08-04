#pragma once

#include <Husky/Vulkan/VkUniquePtr.h>

namespace Husky
{

template<typename Handle>
using VkCreateResult = std::tuple<VkResult, VkUniquePtr<Handle>>;

template<typename T>
using VkEnumerateResult = std::tuple<VkResult, Vector<T>>;

}
