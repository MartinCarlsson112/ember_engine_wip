#pragma once
#include "vulkan\vulkan.h"

static uint32_t find_memory_type(const VkPhysicalDevice& gpu, const uint32_t type_filter, const VkMemoryPropertyFlags properties)
{

	VkPhysicalDeviceMemoryProperties mem_properties;
	vkGetPhysicalDeviceMemoryProperties(gpu, &mem_properties);
	for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
	{
		if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}
	return VK_NULL_HANDLE;
}