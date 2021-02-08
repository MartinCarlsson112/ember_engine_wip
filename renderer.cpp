#include "renderer.h"
#include <iostream>

#include "shader.h"
#include "uniform_buffer_object.h"
#include "vulkan_utils.h"

bool renderer::create_vulkan_context(const char* app_name, const HWND window, const int2 screen_dim)
{
	instance.create(app_name, window);
	surface.create(instance.inst, window);
	if (!device.create(instance.inst, surface.surf))
	{
		return false;
	}
	swapchain.create_swapchain(device, surface.surf, screen_dim.x, screen_dim.y);
	render_pass.create(device.gpu, device.logical_device, swapchain.surface_format.format, em::render_pass_settings());

	device.create_command_pool();
	set_viewport(0, 0, (float)swapchain.extent.width, (float)swapchain.extent.height);
	return true;
}

void renderer::create_swapchain()
{

	swapchain.create_depth_buffers(device);
	swapchain.create_frame_buffers(device.logical_device, render_pass.pass);

	create_uniform_buffer();
	cmd_buffer.create_command_buffers(swapchain.swapchain_frame_buffers, device.logical_device, device.pool);
	//create shadow map
	device.create_image(swapchain.extent.width, swapchain.extent.height, shadow_map, em::image_options{});

	//raytracing
	raytracing.init_raytracing(device);
	raytracing.create_storage_image(device, swapchain.surface_format.format, { swapchain.extent.width, swapchain.extent.height, 1 });
	raytracing.create_pipeline(device);
	raytracing.create_sbt(device);
	raytracing.create_descriptor_set_pools(device, ubos, lbos);
	
	
	semaphores.create(device.logical_device, MAX_FRAMES_IN_FLIGHT);
}

uint32_t renderer::create_pipeline(const em::graphics_pipeline_settings& settings)
{
	graphics_pipeline_settings_storage.push_back(settings);
	graphics_pipelines.push_back(em::graphics_pipeline());
	graphics_pipelines[graphics_pipelines.size() - 1].create(device.logical_device, settings.shaders, descriptor_sets[settings.desc_layout].layout, render_pass.pass, viewport, swapchain.extent, settings);
	return graphics_pipelines.size()-1;
}

uint32_t renderer::create_descriptor_set(const em::descriptor_set_settings& settings)
{
	descriptor_set_settings_storage.push_back(settings);
	descriptor_sets.push_back(em::descriptor_sets());
	descriptor_sets[descriptor_sets.size() - 1].create_pools(device.logical_device, swapchain.swapchain_images);
	descriptor_sets[descriptor_sets.size() - 1].create_layout(device.logical_device);
	descriptor_sets[descriptor_sets.size() - 1].create_sets(device.logical_device, swapchain.swapchain_images, ubos, lbos, pbo, shadow_map.image_view, shadow_map.sampler, settings.materials);
	return descriptor_sets.size() - 1;
}

void renderer::dispose_pipeline()
{
	//window->RemoveWindowMessageListener(this);
	vkDeviceWaitIdle(device.logical_device);
	device.destroy_image(shadow_map);
	cleanup_swapchain();
	for (int i = 0; i < descriptor_sets.size(); i++)
	{
		vkDestroyDescriptorSetLayout(device.logical_device, descriptor_sets[i].layout, nullptr);
	}
	descriptor_sets.clear();

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(device.logical_device, semaphores.render_finished_semaphores[i], nullptr);
		vkDestroySemaphore(device.logical_device, semaphores.image_available_semaphores[i], nullptr);
		vkDestroyFence(device.logical_device, semaphores.in_flight_fences[i], nullptr);
	}
	vkDestroyCommandPool(device.logical_device, device.pool, nullptr);
}

void renderer::dispose_vulkan_context()
{
	vkDeviceWaitIdle(device.logical_device);
	vkDestroyDevice(device.logical_device, nullptr);
	surface.destroy(instance.inst);
	instance.destroy();
}


#pragma warning(push)
#pragma warning(disable : 26812) 


void renderer::render(int2 window_size, light_buffer_object& lbo, camera_data& camera_pos, const std::vector<mesh_batch>& batches, const std::vector<float4x4>& poses)
{
	vkWaitForFences(device.logical_device, 1, &semaphores.in_flight_fences[current_frame], VK_TRUE, std::numeric_limits<uint64_t>::max());
	uint32_t image_index;
	if (!acquire(window_size, image_index))
	{
		return;
	}

	manage_acceleration_structures(batches, camera_pos);
	update_uniform_buffers(image_index, lbo, camera_pos, poses);
	VkSemaphore signal_semaphores[] = { semaphores.render_finished_semaphores[current_frame] };
	draw(image_index, batches);
	vkResetFences(device.logical_device, 1, &semaphores.in_flight_fences[current_frame]);
	submit(image_index, signal_semaphores);
	present(image_index, signal_semaphores);
	swap();
}

#pragma warning(pop)
bool renderer::acquire(int2 window_size, uint32_t &image_index)
{
	VkResult result = vkAcquireNextImageKHR(
		device.logical_device, 
		swapchain.swapchain_khr,
		std::numeric_limits<uint64_t>::max(), 
		semaphores.image_available_semaphores[current_frame], 
		VK_NULL_HANDLE, &image_index);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreate_swapchain(window_size);
		return false;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		std::cout << "acquire image failed or suboptimal" << std::endl;
		return false;
	}
	return true;
}

void renderer::draw(const uint32_t image_index, const std::vector<mesh_batch>& batches)
{
	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = 0;
	begin_info.pInheritanceInfo = nullptr;

	VK_CHECK_RESULT(vkBeginCommandBuffer(cmd_buffer.command_buffers[image_index], &begin_info));
	raytracing.record_command_buffer(device, image_index, cmd_buffer.command_buffers[image_index], batches, swapchain, shadow_map);

	cmd_buffer.record_command_buffer(image_index, swapchain.swapchain_frame_buffers, batches, render_pass.pass, swapchain.extent, descriptor_sets, graphics_pipelines);

	VK_CHECK_RESULT(vkEndCommandBuffer(cmd_buffer.command_buffers[image_index]));
}

void renderer::submit(const uint32_t image_index, const VkSemaphore* signal_semaphore)
{
	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore wait_semaphores[] = { semaphores.image_available_semaphores[current_frame] };

	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait_semaphores;
	submit_info.pWaitDstStageMask = wait_stages;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &cmd_buffer.command_buffers[image_index];
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = signal_semaphore;

	auto res = vkQueueSubmit(device.graphics_queue, 1, &submit_info, semaphores.in_flight_fences[current_frame]);

	if (res != VK_SUCCESS)
	{
		std::cout << "failed to submit queue" << std::endl;
	}
}

void renderer::present(const uint32_t image_index, const VkSemaphore* signal_semaphore)
{
	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signal_semaphore;

	VkSwapchainKHR swapchains[] = { swapchain.swapchain_khr };
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swapchains;

	present_info.pImageIndices = &image_index;

	auto result = vkQueuePresentKHR(device.present_queue, &present_info);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		std::cout << "out of date or suboptimal" << std::endl;
	}
	else if (result != VK_SUCCESS)
	{
		std::cout << "present failed!" << std::endl;
	}
}

void renderer::swap()
{
	last_frame = current_frame;
	current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void renderer::recreate_graphics_pipelines()
{

}

void renderer::create_uniform_buffer()
{
	ubos.resize(swapchain.swapchain_images.size());
	lbos.resize(swapchain.swapchain_images.size());
	pbo.resize(swapchain.swapchain_images.size());
	for (size_t i = 0; i < swapchain.swapchain_images.size(); i++)
	{
		device.create_buffer(ubos[i], sizeof(camera_data), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		device.create_buffer(lbos[i], sizeof(light_buffer_object), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		device.create_buffer(pbo[i], sizeof(float4x4) * 120, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	}
}

void renderer::update_uniform_buffers(const uint32_t current_image, const light_buffer_object& light_data, const camera_data& cam_pos, const std::vector<float4x4>& poses)
{
	void* data;
	vkMapMemory(device.logical_device, ubos[current_image].buffer_memory, 0, sizeof(camera_data), 0, &data);
	memcpy(data, &cam_pos, sizeof(camera_data));
	vkUnmapMemory(device.logical_device, ubos[current_image].buffer_memory);

	vkMapMemory(device.logical_device, lbos[current_image].buffer_memory, 0, sizeof(light_buffer_object), 0, &data);
	memcpy(data, &light_data, sizeof(light_buffer_object));
	vkUnmapMemory(device.logical_device, lbos[current_image].buffer_memory);

	vkMapMemory(device.logical_device, pbo[current_image].buffer_memory, 0, sizeof(float4x4) * poses.size(), 0, &data);
	memcpy(data, poses.data(), sizeof(float4x4) * poses.size());
	vkUnmapMemory(device.logical_device, pbo[current_image].buffer_memory);

}



void renderer::manage_acceleration_structures(const std::vector<mesh_batch>& batch, const camera_data& cam_pos)
{
	//Todo: Optimize acceleration structure rebuilding
	//Todo: check if acceleration structure needs to be rebuilt - avoid if possible
	//check if position - acceleration structure position > threshold
	//check if geometry has changed

	//todo: blas chunking
		//static geometry -> chunks -> blas
			//updated seldom

		//dynamic geo own blas
			//updated often

	if (blas.size() <= 0)
	{
		raytracing.create_blas(device, blas, batch);


		raytracing.create_tlas(device, blas, tlas, batch);
		raytracing.update_descriptor_sets(device, batch, tlas, ubos, lbos);
	}

}

void renderer::recreate_swapchain(int2 window_size)
{
	vkDeviceWaitIdle(device.logical_device);
	cleanup_swapchain();

	swapchain.create_swapchain(device, surface.surf, window_size.x, window_size.y);
	set_viewport(0, 0, (float)swapchain.extent.width, (float)swapchain.extent.height);

	swapchain.create_depth_buffers(device);
	swapchain.create_frame_buffers(device.logical_device, render_pass.pass);
	device.create_command_pool();

	create_uniform_buffer();
	device.create_image(swapchain.extent.width, swapchain.extent.height, shadow_map, em::image_options{});
	for (int i = 0; i < descriptor_set_settings_storage.size(); i++)
	{
		descriptor_sets.push_back(em::descriptor_sets());
		descriptor_sets[descriptor_sets.size() - 1].create_pools(device.logical_device, swapchain.swapchain_images);
		descriptor_sets[descriptor_sets.size() - 1].create_layout(device.logical_device);
		descriptor_sets[descriptor_sets.size() - 1].create_sets(device.logical_device, swapchain.swapchain_images, ubos, lbos, pbo, shadow_map.image_view, shadow_map.sampler, descriptor_set_settings_storage[i].materials);
	}
	for (int i = 0; i < graphics_pipeline_settings_storage.size(); i++)
	{
		graphics_pipelines.push_back(em::graphics_pipeline());
		graphics_pipelines[graphics_pipelines.size() - 1].create(
			device.logical_device,
			graphics_pipeline_settings_storage[i].shaders,
			descriptor_sets[graphics_pipeline_settings_storage[i].desc_layout].layout,
			render_pass.pass,
			viewport,
			swapchain.extent,
			graphics_pipeline_settings_storage[i]);
	}

	


	cmd_buffer.create_command_buffers(swapchain.swapchain_frame_buffers, device.logical_device, device.pool);
	raytracing.create_storage_image(device, swapchain.surface_format.format, {swapchain.extent.width, swapchain.extent.height, 1});
}


void renderer::cleanup_swapchain()
{
	swapchain.depth_object.destroy(device.logical_device);

	for (size_t i = 0; i < swapchain.swapchain_frame_buffers.size(); i++) {
		vkDestroyFramebuffer(device.logical_device, swapchain.swapchain_frame_buffers[i], nullptr);
	}

	blas.clear();

	vkFreeCommandBuffers(device.logical_device, device.pool, static_cast<uint32_t>(cmd_buffer.command_buffers.size()), cmd_buffer.command_buffers.data());

	for (int i = 0; i < graphics_pipelines.size(); i++)
	{
		vkDestroyPipeline(device.logical_device, graphics_pipelines[i].pipeline, nullptr);
		vkDestroyPipelineLayout(device.logical_device, graphics_pipelines[i].layout, nullptr);
	}
	graphics_pipelines.clear();

	device.destroy_image(shadow_map);
	for (size_t i = 0; i < swapchain.swapchain_imageviews.size(); i++) {
		vkDestroyImageView(device.logical_device, swapchain.swapchain_imageviews[i], nullptr);
	}

	vkDestroySwapchainKHR(device.logical_device, swapchain.swapchain_khr, nullptr);

	for (size_t i = 0; i < swapchain.swapchain_images.size(); i++) {
		vkDestroyBuffer(device.logical_device, ubos[i].buffer, nullptr);
		vkFreeMemory(device.logical_device, ubos[i].buffer_memory, nullptr);
	}

	for (int i = 0; i < descriptor_sets.size(); i++)
	{
		vkDestroyDescriptorPool(device.logical_device, descriptor_sets[i].pool, nullptr);
	}
	descriptor_sets.clear();

	raytracing.delete_storage_image(device);
}

void renderer::set_viewport(float x, float y, float width, float height)
{
	viewport = { 0,0, width, height, min_depth, max_depth };
}
