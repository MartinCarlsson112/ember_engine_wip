#include "vulkan\vulkan.h"
#include "depth_object.h"
namespace em
{
	struct render_pass_settings
	{

	};

	class render_pass
	{
	public:
		render_pass() : pass(VK_NULL_HANDLE) {}
		void create(const VkPhysicalDevice& gpu, const VkDevice& device, const VkFormat& format, const render_pass_settings& settings);
		VkRenderPass pass;
	};
}

