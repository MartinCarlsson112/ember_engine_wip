#pragma once
#include <stdint.h>
#include "mmath.h"
#include "animation.h"
constexpr inline int MAX_BATCHED_MESHES_COUNT = 128;
struct skinned_mesh_render_object
{
	pose p;
	float4x4 model;
	VkBuffer vbo = VK_NULL_HANDLE;
	VkBuffer ibo = VK_NULL_HANDLE;
	uint32_t vertex_count = 0;
	uint32_t material = 0;
	uint32_t descriptor_set = 0;
	uint32_t pipeline = 0;
	uint32_t vertex_stride = 0;
};

struct mesh_batch
{
	mesh_batch(): count(0), vbo(VK_NULL_HANDLE), ibo(VK_NULL_HANDLE), vertex_count(0), material(0), descriptor_set(0), pipeline(0){}
	float4x4 model[MAX_BATCHED_MESHES_COUNT]{};
	uint8_t count;
	VkBuffer vbo;
	VkBuffer ibo;
	uint32_t vertex_count;
	uint32_t material;
	uint32_t descriptor_set;
	uint32_t pipeline;
	uint32_t vertex_stride;
};