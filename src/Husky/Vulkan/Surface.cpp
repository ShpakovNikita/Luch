#include <Husky/Vulkan/Surface.h>

namespace Husky::Vulkan
{
    Surface::Surface(vk::Instance aInstance, vk::SurfaceKHR aSurface, vk::AllocationCallbacks aAllocationCallbacks)
        : allocationCallbacks(aAllocationCallbacks)
        , instance(aInstance)
        , surface(aSurface)
    {
    }

    Surface::Surface(Surface && other)
        : allocationCallbacks(other.allocationCallbacks)
        , instance(other.instance)
        , surface(other.surface)
    {
        other.instance = nullptr;
        other.surface = nullptr;
    }

    Surface& Surface::operator=(Surface&& other)
    {
        allocationCallbacks = other.allocationCallbacks;
        instance = other.instance;
        surface = other.surface;

        other.instance = nullptr;
        other.surface = nullptr;

        return *this;
    }

    Surface::~Surface()
    {
        if (instance)
        {
            instance.destroySurfaceKHR(surface, allocationCallbacks);
        }
    }

#ifdef _WIN32
    VulkanResultValue<Surface> Surface::CreateWin32Surface(
        vk::Instance instance,
        HINSTANCE hInstance,
        HWND hWnd,
        const vk::AllocationCallbacks& allocationCallbacks)
    {
        vk::Win32SurfaceCreateInfoKHR ci;
        ci.setHinstance(hInstance);
        ci.setHwnd(hWnd);
        auto [result, vulkanSurface] = instance.createWin32SurfaceKHR(ci, allocationCallbacks);

        if (result == vk::Result::eSuccess)
        {
            auto surface = Surface{ instance, vulkanSurface, allocationCallbacks };
            return { result, std::move(surface) };
        }
        else
        {
            instance.destroySurfaceKHR(vulkanSurface, allocationCallbacks);
            return { result };
        }
    }
#endif

}
