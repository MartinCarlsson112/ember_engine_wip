#pragma once
#include "mmath.h"
#include "vulkan\vulkan.h"
#include <stddef.h>
struct vertex
{
	float4 position;
	float4 normal;
	float2 uv;
};

constexpr  VkVertexInputBindingDescription get_binding_desc()
{
	VkVertexInputBindingDescription binding_desc = {};
	binding_desc.binding = 0;
	binding_desc.stride = sizeof(vertex);
	binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return binding_desc;
};


inline void get_attrib_description(std::vector<VkVertexInputAttributeDescription>&  desc)
{
	desc.push_back(VkVertexInputAttributeDescription());
	desc.push_back(VkVertexInputAttributeDescription());
	desc.push_back(VkVertexInputAttributeDescription());

	desc[0].binding = 0;
	desc[0].location = 0;
	desc[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	desc[0].offset = 0;

	desc[1].binding = 0;
	desc[1].location = 1;
	desc[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	desc[1].offset = offsetof(vertex, normal);

	desc[2].binding = 0;
	desc[2].location = 2;
	desc[2].format = VK_FORMAT_R32G32_SFLOAT;
	desc[2].offset = offsetof(vertex, uv);
}