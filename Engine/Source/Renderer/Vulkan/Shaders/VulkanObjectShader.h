#pragma once

#include "Vulkan/VulkanTypes.inl"
#include "RendererTypes.inl"

bool8 VulkanCreateObjectShader(FVulkanContext* VulkanContext, FVulkanObjectShader* OutShader);

void VulkanReleaseObjectShader(FVulkanContext* VulkanContext, FVulkanObjectShader* Shader);

void VulkanUseObjectShader(FVulkanContext* VulkanContext, FVulkanObjectShader* Shader);