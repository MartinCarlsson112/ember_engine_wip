#pragma once
#include "mmath.h"
#include "vulkan\vulkan.h"
#include <stddef.h>

struct skinned_vertex
{
	float4 position;
	float3 normal;
	float2 uvs;
	float4 bones;
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

struct vertex_attrib_desc
{
	constexpr vertex_attrib_desc() : position(), color() {}

	VkVertexInputAttributeDescription position;
	VkVertexInputAttributeDescription color;
};

constexpr vertex_attrib_desc skinned_vertex_get_attrib_description()
{
	vertex_attrib_desc desc;
	desc.position.binding = 0;
	desc.position.location = 0;
	desc.position.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	desc.position.offset = 0;

	desc.color.binding = 0;
	desc.color.location = 1;
	desc.color.format = VK_FORMAT_R32G32B32_SFLOAT;
	desc.color.offset = offsetof(skinned_vertex, normal);
	return desc;
}