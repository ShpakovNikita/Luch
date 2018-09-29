#pragma once

#include <Husky/Vulkan.h>

namespace Husky::Vulkan
{

/*
class AllocatorUserData
{

}

class VulkanAllocatorExample
{
    void* Allocate(
        AllocatorUserData* userData,
        size_t size,
        size_t alignment,
        VkSystemAllocationScope allocationScope);

    void* Reallocate(
        AllocatorUserData* userData,
        void* pOriginal,
        size_t size,
        size_t alignment,
        VkSystemAllocationScope allocationScope);

    void Free(AllocatorUserData* userData, void* pMemory);

    void InternalAllocationNotification(
        AllocatorUserData* userData,
        size_t size,
        VkInternalAllocationType allocationType,
        VkSystemAllocationScope allocationScope);

    void InternalFreeNotification(
        AllocatorUserData* userData,
        size_t size,
        VkInternalAllocationType allocationType,
        VkSystemAllocationScope allocationScope);
};
*/

template<typename Allocator, typename AllocatorUserData>
class VulkanAllocationDelegate
{
public:
    VulkanAllocationDelegate() = default;

    static void* VKAPI_PTR StaticVulkanAllocationCallback(
        void* pUserData,
        size_t size,
        size_t alignment,
        VkSystemAllocationScope allocationScope)
    {
        UserData* userData = static_cast<UserData*>(pUserData);
        return userData->allocator->Allocate(&userData->allocatorUserData, size, alignment, allocationScope);
    }

    static void* VKAPI_PTR StaticVulkanReallocationCallback(
        void* pUserData,
        void* pOriginal,
        size_t size,
        size_t alignment,
        VkSystemAllocationScope allocationScope)
    {
        UserData* userData = static_cast<UserData*>(pUserData);
        return userData->allocator->Reallocate(&userData->allocatorUserData, pOriginal, size, alignment, allocationScope);
    }

    static void VKAPI_PTR StaticVulkanFreeCallback(
        void* pUserData,
        void* pMemory)
    {
        UserData* userData = static_cast<UserData*>(pUserData);
        return userData->allocator->Free(&userData->allocatorUserData, pMemory);
    }

    static void VKAPI_PTR StaticInternalAllocationNotification(
        void* pUserData,
        size_t size,
        VkInternalAllocationType allocationType,
        VkSystemAllocationScope allocationScope)
    {
        UserData* userData = static_cast<UserData*>(pUserData);
        return userData->allocator->InternalAllocationNotification(&userData->allocatorUserData, size, allocationType, allocationScope);
    }

    static void VKAPI_PTR StaticInternalFreeNotification(
        void* pUserData,
        size_t size,
        VkInternalAllocationType allocationType,
        VkSystemAllocationScope allocationScope)
    {
        UserData* userData = static_cast<UserData*>(pUserData);
        return userData->allocator->InternalFreeNotification(&userData->allocatorUserData, size, allocationType, allocationScope);
    }

    vk::AllocationCallbacks GetAllocationCallbacks()
    {
        vk::AllocationCallbacks allocationCallbacks;
        allocationCallbacks.setPUserData(&userData);
        allocationCallbacks.setPfnAllocation(VulkanAllocationDelegate::StaticVulkanAllocationCallback);
        allocationCallbacks.setPfnReallocation(VulkanAllocationDelegate::StaticVulkanReallocationCallback);
        allocationCallbacks.setPfnFree(VulkanAllocationDelegate::StaticVulkanFreeCallback);
        allocationCallbacks.setPfnInternalAllocation(VulkanAllocationDelegate::StaticInternalAllocationNotification);
        allocationCallbacks.setPfnInternalFree(VulkanAllocationDelegate::StaticInternalFreeNotification);
        return allocationCallbacks;
    }
private:
    struct UserData
    {
        Allocator* allocator;
        AllocatorUserData allocatorUserData;
    };

    UserData userData;
};

}
