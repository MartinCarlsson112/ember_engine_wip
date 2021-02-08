#pragma once
#include "mmath.h"
#include "vulkan\vulkan.h"
#include <stddef.h>

struct skinned_vertex
{
	float4 position;
	float3 normal;
	float2 uvs;
	int4 bones = { -1, -1, -1, -1 };
	float4 weights;
};

constexpr VkVertexInputBindingDescription skinned_vertex_get_binding_desc()
{
	VkVertexInputBindingDescription binding_desc = {};
	binding_desc.binding = 0;
	binding_desc.stride = sizeof(skinned_vertex);
	binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return binding_desc;
};



inline void skinned_vertex_get_attrib_description(std::vector<VkVertexInputAttributeDescription>& desc)
{

	desc.push_back(VkVertexInputAttributeDescription{});
	desc.push_back(VkVertexInputAttributeDescription{});
	desc.push_back(VkVertexInputAttributeDescription{});
	desc.push_back(VkVertexInputAttributeDescription{});
	desc.push_back(VkVertexInputAttributeDescription{});

	desc[0].binding = 0;
	desc[0].location = 0;
	desc[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	desc[0].offset = 0;

	desc[1].binding = 0;
	desc[1].location = 1;
	desc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	desc[1].offset = offsetof(skinned_vertex, normal);

	desc[2].binding = 0;
	desc[2].location = 2;
	desc[2].format = VK_FORMAT_R32G32_SFLOAT;
	desc[2].offset = offsetof(skinned_vertex, uvs);

	desc[3].binding = 0;
	desc[3].location = 3;
	desc[3].format = VK_FORMAT_R32G32B32A32_SINT;
	desc[3].offset = offsetof(skinned_vertex, bones);

	desc[4].binding = 0;
	desc[4].location = 4;
	desc[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	desc[4].offset = offsetof(skinned_vertex, weights);
}