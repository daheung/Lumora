#pragma once

#include "Vulkan/VulkanTypes.inl"

/**
 * Returns the string representation of result.
 * @param Result The result to get the string for.
 * @param bGetExtended Indicates whether to also return an extended result.
 * @reutrns The error code and/or extended error message in string from. Defaults to success for unknown result types.
 */
const char* VulkanResultString(VkResult Result, bool8 bGetExtended);

/**
 * Indicates if the passed result is a success or an error as defined bt the Vulkan spec.
 * @returns True is success; otherwise false. Defaults to true for unknown result types.
 */
bool8 IsVulkanResultSuccess(VkResult Result);