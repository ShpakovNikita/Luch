#include "VulkanAllocator.h"

#if _WIN32
#include <malloc.h>
#endif

void* VulkanAllocatorInternal::Allocate(VulkanAllocatorInternalData* userData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
#ifdef _WIN32
    return _aligned_malloc(size, alignment);
#endif
}

void* VulkanAllocatorInternal::Reallocate(VulkanAllocatorInternalData* userData, void * pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
#ifdef _WIN32
    return _aligned_realloc(pOriginal, size, alignment);
#endif
}

void VulkanAllocatorInternal::Free(VulkanAllocatorInternalData* userData, void * pMemory)
{
#ifdef _WIN32
    return _aligned_free(pMemory);
#endif
}

void VulkanAllocatorInternal::InternalAllocationNotification(VulkanAllocatorInternalData* userData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope)
{
}

void VulkanAllocatorInternal::InternalFreeNotification(VulkanAllocatorInternalData* userData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope)
{
}
