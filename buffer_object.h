#pragma once
#include "vulkan\vulkan.h"

struct buffer_object
{
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory buffer_memory = VK_NULL_HANDLE;
};