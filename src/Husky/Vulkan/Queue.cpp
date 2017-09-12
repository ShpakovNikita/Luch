#include <Husky/Vulkan/Queue.h>

namespace Husky::Vulkan
{
    Queue::Queue(vk::Queue aQueue)
        : queue(aQueue)
    {
    }

    vk::Result Queue::WaitIdle()
    {
        return queue.waitIdle();
    }
}
