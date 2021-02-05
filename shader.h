#pragma once
#include <unordered_map>
#include "vulkan\vulkan.h"
#include "vulkan_utils.h"
#include "device.h"
#include "read_binary_file.h"

namespace em
{
	typedef VkShaderStageFlagBits shader_type;

	struct shader
	{
		constexpr shader() :shader_module(VK_NULL_HANDLE), type(shader_type::VK_SHADER_STAGE_ALL) {}
		shader(VkShaderModule shader_module, shader_type shader_type) : shader_module(shader_module), type(shader_type) {}
		VkShaderModule shader_module;
		shader_type type;
	};

	inline shader load_shader_impl(const device& device, const char* file_path, shader_type type)
	{
		std::vector<char> code = read_binary_file(file_path);
		VkShaderModuleCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		create_info.codeSize = code.size();
		create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());
		VkShaderModule shader_module;
		VK_CHECK_RESULT(vkCreateShaderModule(device.logical_device, &create_info, nullptr, &shader_module));
		return shader(shader_module, type);
	}
		

}



