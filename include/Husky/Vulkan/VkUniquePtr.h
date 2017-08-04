#pragma once

#include <utility>
#include <functional>
#include <vulkan/vulkan.h>

namespace Husky
{

template<typename Handle>
class VkUniquePtr final
{
public:
    using DeleterFunction = std::function<void(Handle, const VkAllocationCallbacks*)>;

    VkUniquePtr() = default;

    VkUniquePtr(Handle aHandle, DeleterFunction aDeleterFunction)
        : deleterFunction(aDeleterFunction)
        , handle(aHandle)
        , allocationCallbacks(nullptr)
    {
    }

    VkUniquePtr(
        Handle aHandle,
        const VkAllocationCallbacks* aAllocationCallbacks,
        DeleterFunction aDeleterFunction)
        : deleterFunction(aDeleterFunction)
        , handle(aHandle)
        , allocationCallbacks(aAllocationCallbacks)
    {
    }

    VkUniquePtr(const VkUniquePtr& other) = delete;

    VkUniquePtr(VkUniquePtr&& other)
        : handle(other.handle)
        , allocationCallbacks(other.allocationCallbacks)
        , deleterFunction(move(other.deleterFunction))
    {
        other.handle = VK_NULL_HANDLE;
        other.allocationCallbacks = nullptr;
    }

    ~VkUniquePtr()
    {
        Release();
    }

    VkUniquePtr& operator=(const VkUniquePtr& other) = delete;
    VkUniquePtr& operator=(VkUniquePtr&& other)
    {
        handle = other.handle;
        allocationCallbacks = other.allocationCallbacks;
        deleterFunction = move(other.deleterFunction);

        other.handle = VK_NULL_HANDLE;
        other.allocationCallbacks = nullptr;
    }

    operator bool() const
    {
        return handle == VK_NULL_HANDLE;
    }

    operator Handle() const
    {
        return handle;
    }

    Handle Get() const
    {
        return handle;
    }

    std::pair<Handle, const VkAllocationCallbacks*> Release()
    {
        auto result = std::make_pair(handle, allocationCallbacks);
        handle = nullptr;
        allocationCallbacks = nullptr;
        return result;
    }

    void Reset()
    {
        if (handle != VK_NULL_HANDLE)
        {
            if(deleterFunction)
            {
                deleterFunction(handle, allocationCallbacks);
            }

            handle = nullptr;
            allocationCallbacks = nullptr;
        }
    }

    Handle* Replace()
    {
        Reset();
        return &handle;
    }
private:
    DeleterFunction deleterFunction;
    Handle handle = VK_NULL_HANDLE;
    const VkAllocationCallbacks* allocationCallbacks = nullptr;
};

}
