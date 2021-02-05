#include "surface.h"
#include <iostream>
#include "vulkan_utils.h"

namespace em
{
	void surface::create(const VkInstance& instance, HWND window)
	{
		VkWin32SurfaceCreateInfoKHR create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		create_info.hwnd = window;
		create_info.hinstance = GetModuleHandleW(nullptr);

		VK_CHECK_RESULT(vkCreateWin32SurfaceKHR(instance, &create_info, nullptr, &surf));
	}

	void surface::destroy(const VkInstance& instance)
	{
		vkDestroySurfaceKHR(instance, surf, nullptr);
	}

}

