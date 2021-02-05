#pragma once
#include <vector>
#include "vulkan/vulkan.h"
struct swapchain_support_details
{
	static swapchain_support_details query_swapchain_support(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);

	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;

};