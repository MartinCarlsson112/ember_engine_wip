#pragma once
#include <stdint.h>
#include "mmath.h"

constexpr inline int MAX_BATCHED_MESHES_COUNT = 128;
struct mesh_batch
{
	mesh_batch(): count(0), vbo(VK_NULL_HANDLE), ibo(VK_NULL_HANDLE), vertex_count(0), texture_id(0), descriptor_set(0) {}
	float4x4 model[MAX_BATCHED_MESHES_COUNT];
	uint8_t count;
	VkBuffer vbo;
	VkBuffer ibo;
	uint32_t vertex_count;
	uint32_t texture_id;
	uint32_t descriptor_set;
};