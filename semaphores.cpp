#include "semaphores.h"
#include "vulkan_utils.h"

namespace em
{
	bool semaphores::create(const VkDevice device, const uint32_t max_frames_in_flight)
	{
		image_available_semaphores.resize(max_frames_in_flight);
		render_finished_semaphores.resize(max_frames_in_flight);
		in_flight_fences.resize(max_frames_in_flight);

		VkSemaphoreCreateInfo semaphore_info = {};
		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fence_info = {};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < max_frames_in_flight; i++)
		{
			VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphore_info, nullptr, &image_available_semaphores[i]));
			VK_CHECK_RESULT(vkCreateFence(device, &fence_info, nullptr, &in_flight_fences[i]));
			VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphore_info, nullptr, &render_finished_semaphores[i]));
		}
		return true;
	}

}
