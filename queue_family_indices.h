#pragma once
#include <optional>

struct queue_family_indices 
{
	std::optional<uint32_t> graphics_family;
	std::optional<uint32_t> present_family;
	
	bool is_complete() 
	{
		return graphics_family.has_value() && present_family.has_value();
	}
};