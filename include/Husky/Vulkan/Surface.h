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

        inline vk::SurfaceKHR GetSurface() { return surface; }
    private:
        Surface(vk::Instance aInstance, vk::SurfaceKHR aSurface, vk::AllocationCallbacks aAllocationCallbacks);
        void Destroy();

        vk::AllocationCallbacks allocationCallbacks;
        vk::Instance instance;
        vk::SurfaceKHR surface;
    };
}
