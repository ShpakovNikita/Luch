#include <Husky/Vulkan/VkException.h>

namespace Husky
{

VkException::VkException(VkResult aResult, std::string aMessage)
    : std::runtime_error(move(aMessage))
    , result(aResult)
{
}

}
