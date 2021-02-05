#include "vulkan\vulkan.h"
#include <vector>

namespace em
{
	struct semaphores
	{
		bool create(const VkDevice device, const uint32_t max_frames_in_flight);
		std::vector<VkSemaphore> image_available_semaphores;
		std::vector<VkSemaphore> render_finished_semaphores;
		std::vector<VkFence> in_flight_fences;
	};
}
