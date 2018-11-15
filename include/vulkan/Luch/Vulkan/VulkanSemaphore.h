#pragma once

#include <Luch/BaseObject.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/VulkanForwards.h>

namespace Luch::Vulkan
{
    class VulkanSemaphore : public BaseObject
    {
        friend class VulkanGraphicsDevice;
    public:
        VulkanSemaphore(VulkanGraphicsDevice* device, vk::Semaphore semaphore);
        ~VulkanSemaphore() override;

        inline vk::Semaphore GetSemaphore() { return semaphore; }
    private:
        void Destroy();

        VulkanGraphicsDevice* device = nullptr;
        vk::Semaphore semaphore;
    };
}
