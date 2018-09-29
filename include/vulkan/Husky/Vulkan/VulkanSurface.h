#pragma once

#include <Husky/Vulkan.h>
#include <Husky/BaseObject.h>

namespace Husky::Vulkan
{
    class VulkanSurface : public BaseObject
    {
    public:
        VulkanSurface(
            vk::Instance aInstance,
            vk::SurfaceKHR aSurface,
            Husky::Optional<vk::AllocationCallbacks> aAllocationCallbacks = {});

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

        Husky::Optional<vk::AllocationCallbacks> allocationCallbacks;
        vk::Instance instance;
        vk::SurfaceKHR surface;
    };
}
