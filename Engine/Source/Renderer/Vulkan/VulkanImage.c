#include "VulkanImage.h"

#include "Vulkan/VulkanDevice.h"
#include "Core/HAL/LumoraMemory.h"
#include "Core/Logger.h"

void VulkanCreateImage(
	FVulkanContext* VulkanContext,
	VkImageType  ImageType, 
	uint32 Width, 
	uint32 Height, 
	VkFormat Format, 
	VkImageTiling Tiling, 
	VkImageUsageFlags Usage, 
	VkMemoryPropertyFlags MemoryFlags, 
	bool8 bCreateView, 
	VkImageAspectFlags ViewAspectFlags, 
	FVulkanImage* OutImage
) {
	/** Copy params */
	OutImage->Width = Width;
	OutImage->Height = Height;

	/** Creation Info. */
	VkImageCreateInfo ImageCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	ImageCreateInfo.extent.width = Width;
	ImageCreateInfo.extent.height = Height;
	ImageCreateInfo.extent.depth = 1;	// TODO: Support configurable depth.
	ImageCreateInfo.mipLevels = 4;		// TODO: Support mip mapping.
	ImageCreateInfo.arrayLayers = 1;	// TODO: Support number of layers in the image.
	ImageCreateInfo.format = Format;
	ImageCreateInfo.tiling = Tiling;
	ImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	ImageCreateInfo.usage = Usage;
	ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;			// TODO: Configurable sample count.
	ImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;	// TODO: Configurable sharing mode.

	VK_CHECK(vkCreateImage(VulkanContext->Device.Device, &ImageCreateInfo, VulkanContext->Allocator, &OutImage->Handle));

	/** Query memory requirements. */
	VkMemoryRequirements MemoryRequirements = { 0 };
	vkGetImageMemoryRequirements(VulkanContext->Device.Device, OutImage->Handle, &MemoryRequirements);

	int32 MemoryType = VulkanContext->FindMemoryIndexFunc(MemoryRequirements.memoryTypeBits, MemoryFlags);
	if (MemoryType == -1)
	{
		LUMORA_ERROR("Required memory type not found. Image not valid.");
	}

	/** Allocate memory */
	VkMemoryAllocateInfo MemoryAllocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	MemoryAllocateInfo.allocationSize = MemoryRequirements.size;
	MemoryAllocateInfo.memoryTypeIndex = MemoryType;
	VK_CHECK(vkAllocateMemory(VulkanContext->Device.Device, &MemoryAllocateInfo, VulkanContext->Allocator, &OutImage->Memory));

	/** Bind the memory */
	VK_CHECK(vkBindImageMemory(VulkanContext->Device.Device, OutImage->Handle, OutImage->Memory, 0));	// TODO: configurable memory offset.

	/** Create view */
	if (bCreateView)
	{
		OutImage->ImageView = 0;
		VulkanCreateImageView(VulkanContext, Format, OutImage, ViewAspectFlags);
	}
}

void VulkanCreateImageView(FVulkanContext* VulkanContext, VkFormat Format, FVulkanImage* Image, VkImageAspectFlags AspectFlags)
{
	VkImageViewCreateInfo ViewCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	ViewCreateInfo.image = Image->Handle;
	ViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;	// TODO: Make configurable.
	ViewCreateInfo.format = Format;
	ViewCreateInfo.subresourceRange.aspectMask = AspectFlags;

	// TODO: Make configurable
	ViewCreateInfo.subresourceRange.baseMipLevel = 0;
	ViewCreateInfo.subresourceRange.levelCount = 1;
	ViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	ViewCreateInfo.subresourceRange.layerCount = 1;

	VK_CHECK(vkCreateImageView(VulkanContext->Device.Device, &ViewCreateInfo, VulkanContext->Allocator, &Image->ImageView));
}

void VulkanReleaseImage(FVulkanContext* VulkanContext, FVulkanImage* Image)
{
	if (Image->ImageView)
	{
		vkDestroyImageView(VulkanContext->Device.Device, Image->ImageView, VulkanContext->Allocator);
		Image->ImageView = NULL;
	}
	if (Image->Memory)
	{
		vkFreeMemory(VulkanContext->Device.Device, Image->Memory, VulkanContext->Allocator);
		Image->Memory = NULL;
	}
	if (Image->Handle)
	{
		vkDestroyImage(VulkanContext->Device.Device, Image->Handle, VulkanContext->Allocator);
		Image->Handle = NULL;
	}
}
