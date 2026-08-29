#pragma once

#include "Vulkan/VulkanTypes.inl"

bool8 CreateShaderModule(FVulkanContext* VulkanContext, const char* Name, const char* TypeStr, VkShaderStageFlagBits ShaderStageFlag, uint32 StageIndex, FVulkanShaderStage* ShaderStages);