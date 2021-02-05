#pragma once
#include "vulkan\vulkan.h"
#include <vector>
#include "Mesh.h"
#include "descriptor_sets.h"
#include "graphics_pipeline.h"
#include "mesh_batch.h"
#include "sprite_batch.h"

namespace em
{
	struct swapchain_command_buffer
	{
		void create_command_buffers(const std::vector<VkFramebuffer>& swap_chain_framebuffers,
			const VkDevice& device,
			const VkCommandPool& command_pool);

		void record_command_buffer(const uint32_t current_image, const std::vector<VkFramebuffer>& swapChainFramebuffers, const std::vector<mesh_batch>& batch,
			const VkRenderPass& render_pass, const VkExtent2D& extents, const descriptor_sets& sets, const graphics_pipeline& graphicsPipeline);

		std::vector<VkCommandBuffer> command_buffers;
	};
}
