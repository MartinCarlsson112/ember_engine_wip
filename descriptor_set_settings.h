#pragma once
#include <vector>
#include "texture.h"
#include "vulkan/vulkan.h"

namespace em
{
	struct descriptor_set_settings
	{
		std::vector<em::texture> materials;
	};
};