#pragma once
#include "vulkan\vulkan.h"
#include <vector>
#include "texture.h"
#include "buffer_object.h"

namespace em
{
	struct descriptor_sets
	{
		descriptor_sets() : layout(VK_NULL_HANDLE), pool(VK_NULL_HANDLE) { }
		VkDescriptorSetLayout layout;
		VkDescriptorPool pool;
		std::vector<VkDescriptorSet> sets;

		void create_layout(const VkDevice& device);
		void create_sets(const VkDevice& device, 
			const std::vector<VkImage>& swapChainImages, 
			const std::vector<buffer_object>& uniformBuffers, 
			const std::vector<buffer_object>& lbos, 
			const VkImageView& shadow_map_view, 
			const VkSampler& shadow_map_sampler,
			const std::vector<em::texture>& textures);
		void create_pools(const VkDevice& device, const std::vector<VkImage>& swapChainImages);
	};

}

