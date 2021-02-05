#pragma once
#include "vulkan\vulkan.h"

void transition_image_layout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, VkDevice device, VkCommandPool command_pool, VkQueue graphics_queue);