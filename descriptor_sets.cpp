#include "descriptor_sets.h"
#include <iostream>
#include "uniform_buffer_object.h"
#include "vulkan_utils.h"
namespace em
{
	void descriptor_sets::create_layout(const VkDevice& device)
	{
		VkDescriptorSetLayoutBinding bindings[4] = {};
		//camera data
		bindings[0].binding = 0;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bindings[0].descriptorCount = 1;
		bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		bindings[0].pImmutableSamplers = nullptr;

		//lbo
		bindings[1].binding = 1;
		bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bindings[1].descriptorCount = 1;
		bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		bindings[1].pImmutableSamplers = nullptr;

		//shadow map
		bindings[2].binding = 2;
		bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		bindings[2].descriptorCount = 1;
		bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		bindings[2].pImmutableSamplers = nullptr;

		//material array
		bindings[3].binding = 3;
		bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		const int MAX_MATERIAL_TEXTURES = 32;
		bindings[3].descriptorCount = MAX_MATERIAL_TEXTURES;
		bindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		bindings[3].pImmutableSamplers = nullptr;


		VkDescriptorSetLayoutCreateInfo layout_info = {};
		layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout_info.bindingCount = COUNT_OF(bindings);
		layout_info.pBindings = bindings;

		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &layout_info, nullptr, &layout));
	}

	void descriptor_sets::create_pools(const VkDevice& device, const std::vector<VkImage>& swapChainImages)
	{
		VkDescriptorPoolSize pool_sizes[4];

		pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		pool_sizes[0].descriptorCount = static_cast<uint32_t>(swapChainImages.size());
		pool_sizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		pool_sizes[1].descriptorCount = static_cast<uint32_t>(swapChainImages.size());
		pool_sizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		pool_sizes[2].descriptorCount = static_cast<uint32_t>(swapChainImages.size());
		pool_sizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		pool_sizes[3].descriptorCount = static_cast<uint32_t>(swapChainImages.size());

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = COUNT_OF(pool_sizes);
		poolInfo.pPoolSizes = pool_sizes;
		poolInfo.maxSets = static_cast<uint32_t>(swapChainImages.size());
		VK_CHECK_RESULT(vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool));
	}

	void descriptor_sets::create_sets(const VkDevice& device, 
		const std::vector<VkImage>& swapchain_images, 
		const std::vector<buffer_object>& ubos, 
		const std::vector<buffer_object>& lbos,
		const VkImageView& shadow_map_view,
		const VkSampler& shadow_map_sampler,
		const std::vector<em::texture>& textures)
	{
		std::vector<VkDescriptorSetLayout> layouts(swapchain_images.size(), layout);
		VkDescriptorSetAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool = pool;
		alloc_info.descriptorSetCount = static_cast<uint32_t>(swapchain_images.size());
		alloc_info.pSetLayouts = layouts.data();

		sets.resize(swapchain_images.size());
		VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &alloc_info, sets.data()));

		VkDescriptorImageInfo descriptor_image_infos[32];
		for (int i = 0; i < 32; i++)
		{
			if (i >= textures.size())
			{
				descriptor_image_infos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				descriptor_image_infos[i].imageView = textures[0].io.image_view;
				descriptor_image_infos[i].sampler = textures[0].io.sampler;
			}
			else
			{
				descriptor_image_infos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				descriptor_image_infos[i].imageView = textures[i].io.image_view;
				descriptor_image_infos[i].sampler = textures[i].io.sampler;
			}

		}

		VkDescriptorImageInfo shadow_map = {};
		shadow_map.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		shadow_map.imageView = shadow_map_view;
		shadow_map.sampler = shadow_map_sampler;


		for (size_t i = 0; i < swapchain_images.size(); i++)
		{
			VkWriteDescriptorSet descriptor_write[4] = {};

			VkDescriptorBufferInfo cam_data = {};
			cam_data.buffer = ubos[i].buffer;
			cam_data.offset = 0;
			cam_data.range = sizeof(camera_data);
			descriptor_write[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_write[0].dstSet = sets[i];
			descriptor_write[0].dstBinding = 0;
			descriptor_write[0].dstArrayElement = 0;
			descriptor_write[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptor_write[0].descriptorCount = 1;
			descriptor_write[0].pBufferInfo = &cam_data;

			VkDescriptorBufferInfo light_data;
			light_data.buffer = lbos[i].buffer;
			light_data.offset = 0;
			light_data.range = sizeof(light_buffer_object);

			descriptor_write[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_write[1].dstSet = sets[i];
			descriptor_write[1].dstBinding = 1;
			descriptor_write[1].dstArrayElement = 0;
			descriptor_write[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptor_write[1].descriptorCount = 1;
			descriptor_write[1].pBufferInfo = &light_data;

			descriptor_write[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_write[2].dstSet = sets[i];
			descriptor_write[2].dstBinding = 2;
			descriptor_write[2].dstArrayElement = 0;
			descriptor_write[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptor_write[2].descriptorCount = 1;
			descriptor_write[2].pImageInfo = &shadow_map;

			descriptor_write[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_write[3].dstSet = sets[i];
			descriptor_write[3].dstBinding = 3;
			descriptor_write[3].dstArrayElement = 0;
			descriptor_write[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptor_write[3].descriptorCount = COUNT_OF(descriptor_image_infos);
			descriptor_write[3].pImageInfo = descriptor_image_infos;

			vkUpdateDescriptorSets(device, COUNT_OF(descriptor_write), descriptor_write, 0, nullptr);
		}
	}
}