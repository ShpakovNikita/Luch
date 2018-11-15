#pragma once

#include <Luch/Vulkan.h>
#include <Luch/BaseObject.h>

namespace Luch::Vulkan
{
    class VulkanSurface : public BaseObject
    {
    public:
        VulkanSurface(
            vk::Instance aInstance,
            vk::SurfaceKHR aSurface,
            Luch::Optional<vk::AllocationCallbacks> aAllocationCallbacks = {});

        ~VulkanSurface();
#ifdef _WIN32
        static VulkanRefResultValue<VulkanSurface> CreateWin32Surface(
            vk::Instance instance,
            HINSTANCE hInstance,
            HWND hWnd,
            const Optional<vk::AllocationCallbacks>& allocationCallbacks);
#endif

#if __APPLE__
        static VulkanRefResultValue<VulkanSurface> CreateMacOSSurface(
            vk::Instance instance,
            void* view);
#endif
        inline vk::SurfaceKHR GetSurface() { return surface; }
    private:
        void Destroy();

        Luch::Optional<vk::AllocationCallbacks> allocationCallbacks;
        vk::Instance instance;
        vk::SurfaceKHR surface;
    };
}
