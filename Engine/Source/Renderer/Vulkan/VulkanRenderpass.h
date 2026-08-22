#pragma once

#include "VulkanTypes.inl"

void VulkanCreateRenderPass(
	FVulkanContext* VulkanContext, 
	FVulkanRenderPass* OutRenderPass, 
	float32 X, float32 Y, float32 Width, float32 Height,
	float32 Red, float32 Green, float32 Blue, float32 Alpha,
	float32 Depth, float32 Stencil
);

void VulkanReleaseRenderPass(FVulkanContext* VulkanContext, FVulkanRenderPass* RenderPass);

void VulkanRenderPassBegin(FVulkanCommandBuffer* CommandBuffer, FVulkanRenderPass* RenderPass, VkFramebuffer FrameBuffer);

void VulkanRenderPassEnd(FVulkanCommandBuffer* CommandBuffer, FVulkanRenderPass* RenderPass);