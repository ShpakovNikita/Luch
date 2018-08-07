#include <Husky/Vulkan/Surface.h>

namespace Husky::Vulkan
{
    Surface::Surface(
        vk::Instance aInstance,
        vk::SurfaceKHR aSurface,
        Husky::Optional<vk::AllocationCallbacks> aAllocationCallbacks)
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
        Destroy();
    }

    void Surface::Destroy()
    {
        if (instance)
        {
            if(allocationCallbacks.has_value())
            {
                instance.destroySurfaceKHR(surface, *allocationCallbacks);
            }
            else
            {
                instance.destroySurfaceKHR(surface);
            }
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

#if __APPLE__
    VulkanResultValue<Surface> Surface::CreateMacOSSurface(
            vk::Instance instance,
            void* view)
    {
        vk::MacOSSurfaceCreateInfoMVK ci;
        ci.setPView(view);

        auto [result, vulkanSurface] = instance.createMacOSSurfaceMVK(ci);
        if(result == vk::Result::eSuccess)
        {
            auto surface = Surface { instance, vulkanSurface };
            return { result, std::move(surface) };
        }
        else
        {
            instance.destroySurfaceKHR(vulkanSurface);
            return { result };
        }
    }
#endif
}
