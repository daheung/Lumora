#pragma once

#include "VulkanTypes.inl"

void VulkanCreateImage(
	FVulkanContext* VulkanContext, 
	VkImageType ImageType, 
	uint32 Width, 
	uint32 Height, 
	VkFormat Format, 
	VkImageTiling Tiling, 
	VkImageUsageFlags Usage, 
	VkMemoryPropertyFlags MemoryFlags, 
	bool8 bCreateView, 
	VkImageAspectFlags ViewAspectFlags, 
	FVulkanImage* OutImage
);

void VulkanCreateImageView(FVulkanContext* VulkanContext, VkFormat Format, FVulkanImage* Image, VkImageAspectFlags AspectFlags);

void VulkanReleaseImage(FVulkanContext* VulkanContext, FVulkanImage* Image);