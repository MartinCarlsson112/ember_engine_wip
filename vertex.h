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

struct vertex_attrib_desc
{
	constexpr vertex_attrib_desc() : position(), normal(), uv() {}

	VkVertexInputAttributeDescription position;
	VkVertexInputAttributeDescription normal;
	VkVertexInputAttributeDescription uv;
};

constexpr vertex_attrib_desc get_attrib_description()
{
	vertex_attrib_desc desc;
	desc.position.binding = 0;
	desc.position.location = 0;
	desc.position.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	desc.position.offset = 0;

	desc.normal.binding = 0;
	desc.normal.location = 1;
	desc.normal.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	desc.normal.offset = offsetof(vertex, normal);

	desc.uv.binding = 0;
	desc.uv.location = 2;
	desc.uv.format = VK_FORMAT_R32G32_SFLOAT;
	desc.uv.offset = offsetof(vertex, uv);
	return desc;
}