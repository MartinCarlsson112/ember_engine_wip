#pragma once
#include "vulkan\vulkan.h"

namespace em
{
	struct storage_image
	{
		storage_image() : image(VK_NULL_HANDLE), image_memory(VK_NULL_HANDLE), imageview(VK_NULL_HANDLE), format(VkFormat::VK_FORMAT_UNDEFINED) {}

		VkImage image = VK_NULL_HANDLE;
		VkDeviceMemory image_memory = VK_NULL_HANDLE;
		VkImageView imageview = VK_NULL_HANDLE;
		VkFormat format = VkFormat::VK_FORMAT_UNDEFINED;
	};
}

