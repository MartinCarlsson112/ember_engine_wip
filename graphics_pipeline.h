#pragma once
#include "vulkan\vulkan.h"
#include "Shader.h"
#include "graphics_pipeline_settings.h"
#include <vector>

namespace em
{
	struct graphics_pipeline
	{
		void create(
			const VkDevice& logical_device, 
			const std::vector<shader>& shaders, 
			const VkDescriptorSetLayout& descriptor_set_layout, 
			const VkRenderPass& render_pass, 
			const VkViewport& viewport, 
			const VkExtent2D extents,
			const graphics_pipeline_settings& settings);

		VkPipelineLayout layout;
		VkPipeline pipeline;
	};
}