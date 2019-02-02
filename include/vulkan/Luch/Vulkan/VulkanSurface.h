#pragma once

#include <Luch/Vulkan.h>
#include <Luch/BaseObject.h>
#include <Luch/Graphics/Surface.h>

namespace Luch::Vulkan
{
    class VulkanSurface : public Luch::Graphics::Surface
    {
    public:
        VulkanSurface(
            vk::Instance aInstance,
            vk::SurfaceKHR aSurface,
            Luch::Optional<vk::AllocationCallbacks> aAllocationCallbacks = {});

        ~VulkanSurface();
#ifdef _WIN32
        static GraphicsResultRefPtr<VulkanSurface> CreateWin32Surface(
            vk::Instance instance,
            HINSTANCE hInstance,
            HWND hWnd,
            const Optional<vk::AllocationCallbacks>& allocationCallbacks);
#endif

#if __APPLE__
        static GraphicsResultRefPtr<VulkanSurface> CreateMacOSSurface(
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
