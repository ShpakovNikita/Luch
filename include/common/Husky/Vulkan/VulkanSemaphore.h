#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Forwards.h>

namespace Husky::Vulkan
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
