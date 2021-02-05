#include "swapchain_command_buffer.h"
#include "vulkan_utils.h"
#include <iostream>

namespace em
{
	void swapchain_command_buffer::create_command_buffers(const std::vector<VkFramebuffer>& swapChainFramebuffers, const VkDevice& logicalDevice,
		const VkCommandPool& pool)
	{
		command_buffers.resize(swapChainFramebuffers.size());

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = pool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)command_buffers.size();

		VK_CHECK_RESULT(vkAllocateCommandBuffers(logicalDevice, &allocInfo, command_buffers.data()))
	}

	void swapchain_command_buffer::record_command_buffer(const uint32_t current_image, 
		const std::vector<VkFramebuffer>& swapchain_framebuffers, const std::vector<mesh_batch>& batches,
		const VkRenderPass& render_pass, const VkExtent2D& extents, 
		const descriptor_sets& sets, const graphics_pipeline& graphics_pipeline)
	{
		VkRenderPassBeginInfo renderpass_info = {};
		renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderpass_info.renderPass = render_pass;
		renderpass_info.framebuffer = swapchain_framebuffers[current_image];
		renderpass_info.renderArea.offset = { 0, 0 };
		renderpass_info.renderArea.extent = extents;
		VkClearValue color_clear_value{};
		color_clear_value.color = { 0.2f, 0.2f, 0.2f, 1.0f };

		VkClearValue depth_clear_value{};
		depth_clear_value.depthStencil = { 1.0f, 0 };

		VkClearValue clear_values[] = {
			color_clear_value,
			depth_clear_value
		};

		renderpass_info.clearValueCount = (uint32_t)COUNT_OF(clear_values);
		renderpass_info.pClearValues = clear_values;

		vkCmdBeginRenderPass(command_buffers[current_image], &renderpass_info, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(command_buffers[current_image], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline.pipeline);

		VkDeviceSize offsets[] = { 0 };
		for (uint32_t j = 0; j < batches.size(); j++)
		{
			vkCmdBindVertexBuffers(command_buffers[current_image], 0, 1, &batches[j].vbo, offsets);
			vkCmdBindDescriptorSets(command_buffers[current_image], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline.layout, 0, 1, &sets.sets[current_image], 0, nullptr);
			for (int k = 0; k < batches[j].count; k++)
			{
				vkCmdPushConstants(command_buffers[current_image], graphics_pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float4x4), &batches[j].model[k]);
				vkCmdPushConstants(command_buffers[current_image], graphics_pipeline.layout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(float4x4), sizeof(uint32_t), &batches[j].texture_id);
				vkCmdDraw(command_buffers[current_image], static_cast<uint32_t>(batches[j].vertex_count), 1, 0, 0);
			}
		}
		vkCmdEndRenderPass(command_buffers[current_image]);
	}

}

