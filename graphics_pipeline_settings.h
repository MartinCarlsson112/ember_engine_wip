#pragma once
#include "vulkan/vulkan.h"
#include <vector>
#include "shader.h"
namespace em
{
	struct graphics_pipeline_settings
	{
		VkVertexInputBindingDescription binding_desc;
		VkPrimitiveTopology primitives;
		VkCullModeFlags cullmode;
		VkFrontFace winding;
		VkSampleCountFlagBits sample_count;
		uint32_t desc_layout;
		std::vector<VkVertexInputAttributeDescription> attribute_desc;
		std::vector<em::shader> shaders;
	};
};
