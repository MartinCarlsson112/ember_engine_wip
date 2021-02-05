#pragma once
#include "vulkan\vulkan.h"
#include "find_memory_type.h"
#include <iostream>
#include "vulkan_utils.h"
#include "device.h"


static void create_image(em::device device ,uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& image_memory) {
    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = width;
    image_info.extent.height = height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.format = format;
    image_info.tiling = tiling;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = usage;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK_RESULT(vkCreateImage(device.logical_device, &image_info, nullptr, &image));

    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(device.logical_device, image, &mem_requirements);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requirements.size;
    alloc_info.memoryTypeIndex = find_memory_type(device.gpu, mem_requirements.memoryTypeBits, properties);

    VK_CHECK_RESULT(vkAllocateMemory(device.logical_device, &alloc_info, nullptr, &image_memory));

    vkBindImageMemory(device.logical_device, image, image_memory, 0);
}