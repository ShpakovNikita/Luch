#pragma once

#include <functional>
#include <tuple>
#include <vulkan/vulkan.h>
#include <Husky/Assert.h>
#include <Husky/Types.h>

namespace Husky
{

template<typename T>
class VkDeleter;

template<>
class VkDeleter<VkInstance>
{
public:
    static constexpr void (*func)(VkInstance, const VkAllocationCallbacks*) = vkDestroyInstance;
};

template<typename Handle>
class VkPtr final
{
public:
    VkPtr() = default;

    VkPtr(Handle aHandle)
        : handle(aHandle)
    {
    }

    VkPtr(Handle aHandle, const VkAllocationCallbacks* aAllocationCallbacks)
        : handle(aHandle)
        , allocationCallbacks(aAllocationCallbacks)
    {
    }

    VkPtr(const VkPtr& other) = delete;

    VkPtr(VkPtr&& other)
        : handle(other.handle)
        , allocationCallbacks(other.allocationCallbacks)
    {
    }

    ~VkPtr()
    {
        Release();
    }

    VkPtr& operator=(const VkPtr& other) = delete;

    operator bool()
    {
        return handle == VK_NULL_HANDLE;
    }

    Handle Release()
    {
        return handle;
    }

    void Reset()
    {
        if (handle != VK_NULL_HANDLE)
        {
            VkDeleter<Handle>::func(handle, allocationCallbacks);
        }
    }

    Handle* Replace()
    {
        Reset();
        return &handle;
    }
private:
    Handle handle = VK_NULL_HANDLE;
    const VkAllocationCallbacks* allocationCallbacks = nullptr;
};

template<typename Handle>
class VkCreateResult
{
public:
    VkCreateResult(VkResult aResult, Handle aObject)
        : result(aResult)
        , object(aObject)
    {
    }

    VkCreateResult(VkResult aResult, Handle aObject, const VkAllocationCallbacks* aAllocationCallbacks)
        : result(aResult)
        , object(aObject, aAllocationCallbacks)
    {
    }

    VkResult result;
    VkPtr<Handle> object;

    template<int32 I>
    decltype(auto) get()
    {
        if constexpr(I == 0)
        {
            return result;
        }
        else if constexpr(I == 1)
        {
            return std::move(object);
        }
    }
};

template<typename Container>
std::tuple<uint32, typename Container::const_pointer> VkArrayParam(const Container& container)
{
    // TODO check if size fits into uint32
    return std::make_tuple(static_cast<uint32>(container.size()), container.data());
}

}
