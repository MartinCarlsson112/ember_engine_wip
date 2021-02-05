#pragma once
#include "vulkan\vulkan.h"
#include "device.h"
#include "depth_object.h"
namespace em
{
	struct swapchain
	{
		swapchain() :swapchain_khr(VK_NULL_HANDLE), extent(VkExtent2D()), surface_format(VkSurfaceFormatKHR())  {}

		void create_swapchain(const em::device& device, const VkSurfaceKHR& surface, const uint32_t& windowRight, const uint32_t& windowBottom);
		void create_depth_buffers(const em::device& device);
		void create_frame_buffers(const VkDevice& device, const VkRenderPass& renderPass);

	private:

		void create_swapchain_images(const VkDevice& device);
		void create_swapchain_imageviews(const VkDevice& device);


		VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR>& presentModes);
		VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities, const uint32_t& windowRight, const uint32_t& windowBottom);

	public:
		VkSwapchainKHR swapchain_khr;
		VkSurfaceFormatKHR surface_format;
		VkExtent2D extent;

		std::vector<VkImage> swapchain_images;
		std::vector<VkImageView> swapchain_imageviews;
		std::vector<VkFramebuffer> swapchain_frame_buffers;
		depth_object depth_object;

	};
}
