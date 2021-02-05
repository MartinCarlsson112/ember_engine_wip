#pragma once
#include "vulkan\vulkan.h"
#include "image_object.h"
#include "device.h"
namespace em
{
	struct texture
	{
		void load(const em::device& device, const char* file_path);
		void destroy(const em::device& device);
		image_object io;
	};
}
