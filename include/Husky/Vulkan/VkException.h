#pragma once

#include <exception>

#include <Husky/Vulkan.h>

namespace Husky
{

class VkException : public std::runtime_error
{
public:
    VkException(VkResult result, std::string message);

    inline VkResult GetResult() const { return result; }
private:
    VkResult result;
};

}