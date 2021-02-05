#include "vulkan\vulkan.h"
#include <windows.h>
#include <vector>


namespace em
{
	struct instance
	{
		instance() : inst(VK_NULL_HANDLE){}

		void create(const char* name, HWND hwnd);
		void destroy();
		VkInstance inst;

	private:
	};
} 
