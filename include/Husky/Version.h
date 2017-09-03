#pragma once

#include <Husky/Vulkan.h>
#include <Husky/Types.h>

namespace Husky
{

struct Version
{
    static auto ToVulkanVersion(const Version& version)
    {
        return VK_MAKE_VERSION(version.major, version.minor, version.patch);
    }

    uint8 major;
    uint8 minor;
    uint8 patch;

    constexpr Version(uint8 aMajor, uint8 aMinor, uint8 aPatch)
        : major(aMajor)
        , minor(aMinor)
        , patch(aPatch)
    {

    }

    Version(const Version& other) = default;
    Version& operator=(const Version& other) = default;

    template<int32 I>
    constexpr decltype(auto) get()
    {
        if constexpr(I == 0)
        {
            return major;
        }
        else if constexpr(I == 1)
        {
            return minor;
        }
        else if constexpr(I == 2)
        {
            return patch;
        }
    }
};

}
