#pragma once

#include <Luch/Vulkan/VulkanAllocationDelegate.h>

struct VulkanAllocatorInternalData
{
};

class VulkanAllocatorInternal
{
public:
   void *Allocate(VulkanAllocatorInternalData* userData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope);
   void *Reallocate(VulkanAllocatorInternalData* userData, void * pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope);
   void Free(VulkanAllocatorInternalData* userData, void * pMemory);
   void InternalAllocationNotification(VulkanAllocatorInternalData* userData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope);
   void InternalFreeNotification(VulkanAllocatorInternalData* userData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope);
};

class VulkanAllocator : public Luch::Vulkan::VulkanAllocationDelegate<VulkanAllocatorInternal, VulkanAllocatorInternalData>
{
};
