#pragma once

#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;

    class Semaphore
    {
        friend class GraphicsDevice;
    public:
        Semaphore() = default;

        Semaphore(Semaphore&& other);
        Semaphore& operator=(Semaphore&& other);

        ~Semaphore();

        inline vk::Semaphore GetSemaphore() { return semaphore; }
    private:
        Semaphore(GraphicsDevice* device, vk::Semaphore semaphore);
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::Semaphore semaphore;
    };
}
