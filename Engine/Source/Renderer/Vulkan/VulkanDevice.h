#include "Defines.h"

#include "VulkanTypes.inl"

bool8 VulkanCreateDevice(FVulkanContext* VulkanContext);

void VulkanReleaseDevice(FVulkanContext* VulkanContext);

void VulkanDeviceQuerySwapchainSupport(VkPhysicalDevice PhysicalDevice, VkSurfaceKHR Surface, FVulkanSwapchainSupportInfo* OutSupportInfo);