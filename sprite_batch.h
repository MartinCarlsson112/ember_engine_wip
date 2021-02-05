#pragma once
#include "vulkan/vulkan.h"

struct sprite_batch
{
	VkBuffer vbo;
	VkBuffer staging_buffer;
	VkDeviceSize buffer_size;
	uint32_t vertex_count;
};