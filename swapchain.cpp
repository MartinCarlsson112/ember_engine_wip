#include "swapchain.h"
#include "swapchain_support_details.h"
#include "vulkan_utils.h"

namespace em
{

	void swapchain::create_depth_buffers(const em::device& device)
	{
		depth_object.create(device, extent);
	}

	void swapchain::create_swapchain(const em::device& device, const VkSurfaceKHR& surface, const uint32_t& windowRight, const uint32_t& windowBottom)
	{
		swapchain_support_details::query_swapchain_support(device.gpu, surface);
		surface_format = choose_swap_surface_format(device.swapchain_support.formats);
		extent = choose_swap_extent(device.swapchain_support.capabilities, windowRight, windowBottom);
		uint32_t queue_family_indices[] = { device.indices.graphics_family.value(), device.indices.present_family.value() };
		VkPresentModeKHR present_mode = choose_swap_present_mode(device.swapchain_support.presentModes);
		uint32_t image_count = device.swapchain_support.capabilities.minImageCount +1;
		if (image_count > device.swapchain_support.capabilities.maxImageCount)
		{
			image_count = device.swapchain_support.capabilities.maxImageCount;
		}
		VkSwapchainCreateInfoKHR create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		create_info.surface = surface;
		create_info.minImageCount = image_count;
		create_info.imageFormat = surface_format.format;
		create_info.imageColorSpace = surface_format.colorSpace;
		create_info.imageExtent = extent;
		create_info.imageArrayLayers = 1;
		create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		create_info.preTransform = device.swapchain_support.capabilities.currentTransform;
		create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		create_info.presentMode = present_mode;
		create_info.clipped = VK_TRUE;
		create_info.oldSwapchain = VK_NULL_HANDLE;

		if (queue_family_indices[0] != queue_family_indices[1])
		{
			create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			create_info.queueFamilyIndexCount = 2;
			create_info.pQueueFamilyIndices = queue_family_indices;
		}
		else
		{
			create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			create_info.queueFamilyIndexCount = 0;
			create_info.pQueueFamilyIndices = nullptr;
		}
		VK_CHECK_RESULT(vkCreateSwapchainKHR(device.logical_device, &create_info, nullptr, &swapchain_khr));
		create_swapchain_images(device.logical_device);
		create_swapchain_imageviews(device.logical_device);
	}

	void swapchain::create_swapchain_images(const VkDevice& device)
	{
		uint32_t imageCount = 0;
		vkGetSwapchainImagesKHR(device, swapchain_khr, &imageCount, nullptr);
		swapchain_images.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapchain_khr, &imageCount, swapchain_images.data());
	}

	void swapchain::create_swapchain_imageviews(const VkDevice& device)
	{
		swapchain_imageviews.resize(swapchain_images.size());
		for (size_t i = 0; i < swapchain_images.size(); i++)
		{
			VkImageViewCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapchain_images[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = surface_format.format;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;
			VK_CHECK_RESULT(vkCreateImageView(device, &createInfo, nullptr, &swapchain_imageviews[i]))
		}
	}

	void swapchain::create_frame_buffers(const VkDevice& device, const VkRenderPass& renderPass)
	{
		swapchain_frame_buffers.resize(swapchain_imageviews.size());
		for (size_t i = 0; i < swapchain_imageviews.size(); i++) {

			VkImageView attachments[] = {
					swapchain_imageviews[i],
					depth_object.storage.imageview
			};
					
			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = COUNT_OF(attachments);
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = extent.width;
			framebufferInfo.height = extent.height;
			framebufferInfo.layers = 1;
			VK_CHECK_RESULT(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapchain_frame_buffers[i]));
		}
	}

	VkSurfaceFormatKHR swapchain::choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}
		return availableFormats[0];
	}

	VkPresentModeKHR swapchain::choose_swap_present_mode(const std::vector<VkPresentModeKHR>& presentModes)
	{
		for (const auto& availablePresentMode : presentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D swapchain::choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities, const uint32_t& windowRight, const uint32_t& windowBottom)
	{
		//TODO: Fix this, maxImageExtent and MinImageExtent lies... 
		VkExtent2D actualExtent = { (uint32_t)windowRight , (uint32_t)windowBottom };
		//actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		//actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		return actualExtent;
	}
}
