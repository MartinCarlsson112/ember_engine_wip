#include "vulkan\vulkan.h"

namespace em
{
	struct surface
	{
		surface() : surf(VK_NULL_HANDLE) {}

		void create(const VkInstance& instance, HWND window);
		void destroy(const VkInstance& instance);
		VkSurfaceKHR surf;
	};
}
