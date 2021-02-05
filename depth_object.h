#pragma once
#include "vulkan\vulkan.h"
#include "device.h"
#include "storage_image.h"

namespace em
{
	struct depth_object
	{
		depth_object() { storage.format = VK_FORMAT_A1R5G5B5_UNORM_PACK16; }

		em::storage_image storage;

		void create(const em::device& device, VkExtent2D extents);
		void destroy(VkDevice device);
	};
}
