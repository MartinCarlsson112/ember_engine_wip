#pragma once
#include "vulkan\vulkan.h"
#include "buffer_object.h"

namespace em
{
	struct image_options
	{
		VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
		VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
		VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	};

	struct image_object
	{
		VkImage image = VK_NULL_HANDLE;
		VkDeviceMemory image_memory = VK_NULL_HANDLE;
		VkImageView image_view = VK_NULL_HANDLE;
		VkSampler sampler = VK_NULL_HANDLE;
	};
}
