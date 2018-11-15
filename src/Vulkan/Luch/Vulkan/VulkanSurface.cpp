#include <Luch/Vulkan/VulkanSurface.h>

namespace Luch::Vulkan
{
    VulkanSurface::VulkanSurface(
        vk::Instance aInstance,
        vk::SurfaceKHR aSurface,
        Luch::Optional<vk::AllocationCallbacks> aAllocationCallbacks)
        : allocationCallbacks(aAllocationCallbacks)
        , instance(aInstance)
        , surface(aSurface)
    {
    }

    VulkanSurface::~VulkanSurface()
    {
        Destroy();
    }

    void VulkanSurface::Destroy()
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
    VulkanRefResultValue<VulkanSurface> VulkanSurface::CreateWin32Surface(
        vk::Instance instance,
        HINSTANCE hInstance,
        HWND hWnd,
        const Optional<vk::AllocationCallbacks>& allocationCallbacks)
    {
        vk::Win32SurfaceCreateInfoKHR ci;
        ci.setHinstance(hInstance);
        ci.setHwnd(hWnd);

        vk::Optional<const vk::AllocationCallbacks> callbacks = nullptr;
        if (allocationCallbacks.has_value())
        {
            callbacks = *allocationCallbacks;
        }

        auto [result, vulkanSurface] = instance.createWin32SurfaceKHR(ci, callbacks);

        if (result == vk::Result::eSuccess)
        {
            auto surface = MakeRef<VulkanSurface>(instance, vulkanSurface, allocationCallbacks);
            return { result, std::move(surface) };
        }
        else
        {
            instance.destroySurfaceKHR(vulkanSurface, callbacks);
            return { result };
        }
    }
#endif

#if __APPLE__
    VulkanRefResultValue<VulkanSurface> VulkanSurface::CreateMacOSSurface(
        vk::Instance instance,
        void* view)
    {
        vk::MacOSSurfaceCreateInfoMVK ci;
        ci.setPView(view);

        auto [result, vulkanSurface] = instance.createMacOSSurfaceMVK(ci);
        if(result == vk::Result::eSuccess)
        {
            auto surface = MakeRef<VulkanSurface>(instance, vulkanSurface);
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
