#include "instance.h"
#include "vulkan\vulkan_core.h"
#include <iostream>
#include "vk_extensions.h"
#include "vulkan_utils.h"

#include <string.h>
#include <sstream>

namespace em
{
	void instance::create(const char* name, HWND hwnd)
	{
		VkApplicationInfo app_info = {};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.apiVersion =  VK_API_VERSION_1_2;
		app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.pApplicationName = name;
		app_info.pEngineName = name;

		VkInstanceCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		create_info.pApplicationInfo = &app_info;
		
#ifdef _DEBUG
		create_info.enabledExtensionCount = (uint32_t)enabled_extensions.size();
		create_info.ppEnabledExtensionNames = enabled_extensions.data();
		const char* validationLayers[] = { "VK_LAYER_KHRONOS_validation"};
		create_info.enabledLayerCount = 1;
		create_info.ppEnabledLayerNames = validationLayers;
#else
		//Note to Self: Remember to set enabled extensions count if this array is ever changed!
		const char* enabled_extensions[] = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
		create_info.enabledExtensionCount = 2U;
		create_info.ppEnabledExtensionNames = enabled_extensions;
		create_info.enabledLayerCount = 0;
#endif // _DEBUG

		VK_CHECK_RESULT(vkCreateInstance(&create_info, 0, &inst))
	}

	void instance::destroy()
	{
		vkDestroyInstance(inst, nullptr);
	}
}