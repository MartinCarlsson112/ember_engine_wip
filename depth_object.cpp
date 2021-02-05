#include "depth_object.h"
#include "find_supported_format.h"
#include "create_image.h"
#include "vulkan_utils.h"

namespace em
{
	void depth_object::create(const em::device& device, VkExtent2D extents)
	{
		storage.format = find_depth_format(device.gpu);
		create_image(device, extents.width, extents.height,
			storage.format,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			storage.image, storage.image_memory);

		VkImageViewCreateInfo view_info{};
		view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_info.image = storage.image;
		view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view_info.format = storage.format;
		view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		view_info.subresourceRange.baseMipLevel = 0;
		view_info.subresourceRange.levelCount = 1;
		view_info.subresourceRange.baseArrayLayer = 0;
		view_info.subresourceRange.layerCount = 1;

		VK_CHECK_RESULT(vkCreateImageView(device.logical_device, &view_info, nullptr, &storage.imageview));
	}

	void depth_object::destroy(VkDevice device)
	{
		vkDestroyImageView(device, storage.imageview, nullptr);
		vkDestroyImage(device, storage.image, nullptr);
		vkFreeMemory(device, storage.image_memory, nullptr);
	}
}
