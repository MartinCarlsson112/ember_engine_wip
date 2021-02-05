#pragma once
#include "vulkan\vulkan.h"
#include "Vertex.h"
#include "buffer_object.h"
#include <vector>

struct mesh
{
	buffer_object vbo;
	uint32_t vertex_count = 0;
};