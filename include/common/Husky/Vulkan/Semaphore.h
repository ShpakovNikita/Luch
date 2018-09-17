#pragma once

#include <Husky/BaseObject.h>
#include <Husky/Vulkan.h>
#include <Husky/Vulkan/Forwards.h>

namespace Husky::Vulkan
{
    class GraphicsDevice;

    class Semaphore : public BaseObject
    {
        friend class GraphicsDevice;
    public:
        Semaphore(GraphicsDevice* device, vk::Semaphore semaphore);
        ~Semaphore() override;

        inline vk::Semaphore GetSemaphore() { return semaphore; }
    private:
        void Destroy();

        GraphicsDevice* device = nullptr;
        vk::Semaphore semaphore;
    };
}
