#pragma once

#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{
    class Surface
    {
    public:
        Surface() = default;

        Surface(Surface&& other);
        Surface& operator=(Surface&& other);

        ~Surface();
#ifdef _WIN32
        static VulkanResultValue<Surface> CreateWin32Surface(
            vk::Instance instance,
            HINSTANCE hInstance,
            HWND hWnd,
            const vk::AllocationCallbacks& allocationCallbacks);
#endif

#if __APPLE__
        static VulkanResultValue<Surface> CreateMacOSSurface(
            vk::Instance instance,
            void* view);
#endif
        inline vk::SurfaceKHR GetSurface() { return surface; }
    private:
        Surface(
            vk::Instance aInstance,
            vk::SurfaceKHR aSurface,
            Husky::Optional<vk::AllocationCallbacks> aAllocationCallbacks = {});
        void Destroy();

        Husky::Optional<vk::AllocationCallbacks> allocationCallbacks;
        vk::Instance instance;
        vk::SurfaceKHR surface;
    };
}
