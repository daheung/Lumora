#include "VulkanSwapchain.h"

#include "Core/Logger.h"
#include "Core/Misc/Math.h"
#include "Core/HAL/LumoraMemory.h"
#include "Vulkan/VulkanDevice.h"

static void VulkanCreateSwapchainImpl(FVulkanContext* VulkanContext, uint32 Width, uint32 Height, FVulkanSwapchain* Swapchain);
static void VulkanReleaseSwapchainImpl(FVulkanContext* VulkanContext, FVulkanSwapchain* Swapchain);

void VulkanCreateSwapchain(FVulkanContext* VulkanContext, uint32 Width, uint32 Height, FVulkanSwapchain* OutSwapchain)
{
	VulkanCreateSwapchainImpl(VulkanContext, Width, Height, OutSwapchain);
}

void VulkanRecreateSwapchain(FVulkanContext* VulkanContext, uint32 Width, uint32 Height, FVulkanSwapchain* Swapchain)
{
	VulkanReleaseSwapchainImpl(VulkanContext, Swapchain);
	VulkanCreateSwapchainImpl(VulkanContext, Width, Height, Swapchain);
}

void VulkanReleaseSwapchain(FVulkanContext* VulkanContext, uint32 Width, uint32 Height, FVulkanSwapchain* Swapchain)
{
	VulkanReleaseSwapchainImpl(VulkanContext, Swapchain);
}

bool8 VulkanSwapchainAcquireNextImageIndex(FVulkanContext* VulkanContext, FVulkanSwapchain* Swapchain, uint64 TimeoutNs, VkSemaphore ImageAvailableSemaphore, VkFence Fence, uint32* OutImageIndex)
{
	VkResult Result = vkAcquireNextImageKHR(VulkanContext->Device.Device, Swapchain->Handle, TimeoutNs, ImageAvailableSemaphore, Fence, OutImageIndex);
	if (Result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		/** Trigger swapchain recreation, then boot out of the render loop. */
		VulkanRecreateSwapchain(VulkanContext, VulkanContext->FrameBufferWidth, VulkanContext->FrameBufferHeight, Swapchain);
		return FALSE;
	}
	else if (Result != VK_SUCCESS && Result != VK_SUBOPTIMAL_KHR)
	{
		LUMORA_FATAL("Failed to acquire swapchain image.");
		return FALSE;
	}

	return TRUE;
}

void VulkanSwapchainPresent(FVulkanContext* VulkanContext, FVulkanSwapchain* Swapchain, VkQueue GraphicsQueue, VkQueue PresentQueue, VkSemaphore RenderCompleteSemaphore, uint32 PresentImageIndex)
{
	/** Return the image to the swapchain for presentation. */
	VkPresentInfoKHR PresentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
	PresentInfo.waitSemaphoreCount = 1;
	PresentInfo.pWaitSemaphores = &RenderCompleteSemaphore;
	PresentInfo.swapchainCount = 1;
	PresentInfo.pSwapchains = &Swapchain->Handle;
	PresentInfo.pImageIndices = &PresentImageIndex;
	PresentInfo.pResults = 0;

	VkResult Result = vkQueuePresentKHR(PresentQueue, &PresentInfo);
	if (Result == VK_ERROR_OUT_OF_DATE_KHR || Result == VK_SUBOPTIMAL_KHR)
	{
		/** Swapchain is out of date, suboptimal or a framebuffer resize has occurred. Trigger swapchain recreation. */
		VulkanRecreateSwapchain(VulkanContext, VulkanContext->FrameBufferWidth, VulkanContext->FrameBufferHeight, Swapchain);
	}
	else if (Result != VK_SUCCESS)
	{
		LUMORA_FATAL("Failed to present swapchain image.");
	}
}

void VulkanCreateSwapchainImpl(FVulkanContext* VulkanContext, uint32 Width, uint32 Height, FVulkanSwapchain* Swapchain)
{
	VkExtent2D SwapchainExtent = { Width, Height };
	Swapchain->MaxFramesInFlight = 2;

	/** Choose a swap surface format. */
	bool8 bFound = FALSE;
	for (uint32 I = 0; I < VulkanContext->Device.SwapchainSupport.FormatCount; ++I)
	{
		VkSurfaceFormatKHR Format = VulkanContext->Device.SwapchainSupport.Formats[I];
		/** Preferred formats */
		const bool8 bRGBA8UNormalizedFormat = Format.format == VK_FORMAT_B8G8R8A8_UNORM;
		const bool8 bRGBNonlinearColor = Format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		if (bRGBA8UNormalizedFormat && bRGBNonlinearColor)
		{
			Swapchain->ImageFormat = Format;
			bFound = TRUE;
			break;
		}
	}

	if (!bFound)
	{
		Swapchain->ImageFormat = VulkanContext->Device.SwapchainSupport.Formats[0];
	}

	VkPresentModeKHR PresentMode = VK_PRESENT_MODE_FIFO_KHR;
	for (uint32 I = 0; I < VulkanContext->Device.SwapchainSupport.PresentModeCount; ++I)
	{
		VkPresentModeKHR LocalPresentMode = VulkanContext->Device.SwapchainSupport.PresentModes[I];
		if (LocalPresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			PresentMode = LocalPresentMode;
			break;
		}
	}

	/** Requery swapchain support. */
	VulkanDeviceQuerySwapchainSupport(VulkanContext->Device.Device, VulkanContext->Surface, &VulkanContext->Device.SwapchainSupport);

	/** Swapchain extent */
	const bool8 bIsValidWidth  = VulkanContext->Device.SwapchainSupport.Capabilities.currentExtent.width  != UINT32_MAX;
	const bool8 bIsValidHeight = VulkanContext->Device.SwapchainSupport.Capabilities.currentExtent.height != UINT32_MAX;
	if (bIsValidWidth && bIsValidHeight)
	{
		SwapchainExtent = VulkanContext->Device.SwapchainSupport.Capabilities.currentExtent;
	}

	const VkExtent2D Min = VulkanContext->Device.SwapchainSupport.Capabilities.minImageExtent;
	const VkExtent2D Max = VulkanContext->Device.SwapchainSupport.Capabilities.maxImageExtent;
	SwapchainExtent.width  = CMathClamp(SwapchainExtent.width , Min.width , Max.width);
	SwapchainExtent.height = CMathClamp(SwapchainExtent.height, Min.height, Max.height);

	uint32 ImageCount = VulkanContext->Device.SwapchainSupport.Capabilities.minImageCount + 1;
	if (VulkanContext->Device.SwapchainSupport.Capabilities.maxImageCount > 0 && ImageCount > VulkanContext->Device.SwapchainSupport.Capabilities.maxImageCount)
	{
		ImageCount = VulkanContext->Device.SwapchainSupport.Capabilities.maxImageCount;
	}

	/** Swapchain create info */
	VkSwapchainCreateInfoKHR SwapchainCreateInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
	SwapchainCreateInfo.surface = VulkanContext->Surface;
	SwapchainCreateInfo.minImageCount = ImageCount;
	SwapchainCreateInfo.imageFormat = Swapchain->ImageFormat.format;
	SwapchainCreateInfo.imageColorSpace = Swapchain->ImageFormat.colorSpace;
	SwapchainCreateInfo.imageExtent = SwapchainExtent;
	SwapchainCreateInfo.imageArrayLayers = 1;
	SwapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	/** Setup the queue family indices. */
	if (VulkanContext->Device.GraphicsQueueIndex != VulkanContext->Device.PresentQueueIndex)
	{
		uint32 QueueFamilyIndices[] = { (uint32)VulkanContext->Device.GraphicsQueueIndex, (uint32)VulkanContext->Device.PresentQueueIndex };
		SwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		SwapchainCreateInfo.queueFamilyIndexCount = 2;
		SwapchainCreateInfo.pQueueFamilyIndices = QueueFamilyIndices;
	}
	else
	{
		SwapchainCreateInfo.imageArrayLayers = VK_SHARING_MODE_EXCLUSIVE;
		SwapchainCreateInfo.queueFamilyIndexCount = 0;
		SwapchainCreateInfo.pQueueFamilyIndices = NULL;
	}

	SwapchainCreateInfo.preTransform = VulkanContext->Device.SwapchainSupport.Capabilities.currentTransform;
	SwapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	SwapchainCreateInfo.presentMode = PresentMode;
	SwapchainCreateInfo.clipped = VK_TRUE;
	SwapchainCreateInfo.oldSwapchain = NULL;

	VK_CHECK(vkCreateSwapchainKHR(VulkanContext->Device.Device, &SwapchainCreateInfo, VulkanContext->Allocator, &Swapchain->Handle));

	/** Start with a zero frame index. */
	VulkanContext->CurrentFrame = 0;

	/** Images */
	Swapchain->ImageCount = 0;

	VK_CHECK(vkGetSwapchainImagesKHR(VulkanContext->Device.Device, Swapchain->Handle, &Swapchain->ImageCount, NULL));
	if (!Swapchain->Images)
	{
		const uint32 ImagesAllocSize = sizeof(VkImage) * Swapchain->ImageCount;
		Swapchain->Images = (VkImage*)HAllocate(ImagesAllocSize, MEMORY_TAG_RENDERER);
	}
	if (!Swapchain->ImageViews)
	{
		const uint32 ViewsAllocSize = sizeof(VkImageView) * Swapchain->ImageCount;
		Swapchain->ImageViews = (VkImageView*)HAllocate(ViewsAllocSize, MEMORY_TAG_RENDERER);
	}
	VK_CHECK(vkGetSwapchainImagesKHR(VulkanContext->Device.Device, Swapchain->Handle, &Swapchain->ImageCount, Swapchain->Images));

	/** Create image views. */
	for (uint32 I = 0; I < Swapchain->ImageCount; ++I)
	{
		VkImageViewCreateInfo ImageViewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		ImageViewInfo.image = Swapchain->Images[I];
		ImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ImageViewInfo.format = Swapchain->ImageFormat.format;
		ImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ImageViewInfo.subresourceRange.baseMipLevel = 0;
		ImageViewInfo.subresourceRange.levelCount = 1;
		ImageViewInfo.subresourceRange.baseArrayLayer = 0;
		ImageViewInfo.subresourceRange.layerCount = 1;

		VK_CHECK(vkCreateImageView(VulkanContext->Device.Device, &ImageViewInfo, VulkanContext->Allocator, &Swapchain->ImageViews[I]));
	}

	/** Depth resources */
	if (!VulkanDeviceDetectDepthFormat(&VulkanContext->Device))
	{
		VulkanContext->Device.DepthFormat = VK_FORMAT_UNDEFINED;
		LUMORA_FATAL("Failed to find a support depth format.");
	}


}

void VulkanReleaseSwapchainImpl(FVulkanContext* VulkanContext, FVulkanSwapchain* Swapchain)
{
}
