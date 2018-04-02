#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;

    class Semaphore : public BaseObject
    {
        friend class GraphicsDevice;
    public:
        Semaphore(GraphicsDevice* device, vk::Semaphore semaphore);

        Semaphore(Semaphore&& other) = delete;
        Semaphore(const Semaphore& other) = delete;
        Semaphore& operator=(const Semaphore& other) = delete;
        Semaphore& operator=(Semaphore&& other) = delete;

        ~Semaphore() override;

        inline vk::Semaphore GetSemaphore() { return semaphore; }
    private:
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::Semaphore semaphore;
    };
}
