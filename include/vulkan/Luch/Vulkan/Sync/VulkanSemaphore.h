#pragma once

#include <Luch/BaseObject.h>
#include <Luch/Vulkan.h>
#include <Luch/Vulkan/Common/VulkanForwards.h>
#include <Luch/Graphics/Semaphore.h>

namespace Luch::Vulkan
{
    class VulkanSemaphore : public Luch::Graphics::Semaphore
    {
        friend class VulkanGraphicsDevice;
    public:
        VulkanSemaphore(VulkanGraphicsDevice* device, vk::Semaphore semaphore);
        ~VulkanSemaphore() override;

        bool Wait(Optional<int64> timeoutNS = {}) override;
        void Signal() override;

        inline vk::Semaphore GetSemaphore() { return semaphore; }
    private:
        void Destroy();

        VulkanGraphicsDevice* device = nullptr;
        vk::Semaphore semaphore;
    };
}
