#pragma once

#include "Vulkan/VulkanTypes.inl"

void VulkanCreateFrameBuffer(
	FVulkanContext* VulkanContext, 
	FVulkanRenderPass* RenderPass, 
	uint32 Width, uint32 Height, 
	uint32 AttachmentCount, 
	VkImageView* Attachments, 
	FVulkanFrameBuffer* OutFrameBuffer
);

void VulkanReleaseFrameBuffer(FVulkanContext* VulkanContext, FVulkanFrameBuffer* FrameBuffer);