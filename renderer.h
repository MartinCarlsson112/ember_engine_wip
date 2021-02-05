#pragma once
#include "vulkan\vulkan.h"
#include "instance.h"
#include "surface.h"
#include "device.h"
#include "swapchain.h"
#include "render_pass.h"
#include "descriptor_sets.h"
#include "graphics_pipeline.h"
#include "swapchain_command_buffer.h"
#include "mesh.h"
#include "semaphores.h"
#include "resource_manager.h"
#include "depth_object.h"
#include "mesh_batch.h"
#include "uniform_buffer_object.h"
#include "sprite_batch.h"
#include "raytracing.h"
#include "mesh_batch.h"
#include <windows.h>
#undef max
#undef min
#include "mmath.h"
constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;


struct renderer
{
	renderer() : current_frame(0), last_frame(0), blas(), tlas(), viewport(), pipeline(), shadow_map(), max_depth(1.0f), min_depth(0.0f){ }

	bool create_vulkan_context(const char* app_name, const HWND window, const int2 screen_dim);

	//TODO: Parallelize memory allocation, image/buffer creation
	void create_pipeline(std::vector<em::shader>* shaders, std::vector<em::texture>* materials);

	void render(int2 window_size, light_buffer_object& lbo, camera_data& camera_pos, const std::vector<mesh_batch>& batches);

	void dispose_pipeline();
	void dispose_vulkan_context();
	em::device device;
private:

	void recreate_swapchain(int2 window_size);
	void cleanup_swapchain();

	void set_viewport(float x, float y, float width, float height);

	void create_uniform_buffer();
	void update_uniform_buffers(const uint32_t current_image, const light_buffer_object& light_data, const camera_data& cam_pos);

	void manage_acceleration_structures(const std::vector<mesh_batch>& batch, const camera_data& cam_pos);

	bool acquire(const int2 window_size, uint32_t& image_index);

	//TODO: Parallelize command buffer recording, descriptor set updates
	void draw(const uint32_t image_index, const std::vector<mesh_batch>& batches);

	void submit(const uint32_t image_index, const VkSemaphore* signal_semaphore);

	void present(const uint32_t image_index, const VkSemaphore* signal_semaphore);

	void swap();

	std::vector<buffer_object> ubos;
	std::vector<buffer_object> lbos;
	std::vector<em::shader>* shaders;

	std::vector<em::texture>* textures;

	em::image_object shadow_map;

	em::instance instance;

	em::render_pass render_pass;
	em::surface surface;
	em::swapchain swapchain;
	em::descriptor_sets descriptor_sets;
	em::graphics_pipeline pipeline;
	em::swapchain_command_buffer cmd_buffer;
	em::semaphores semaphores;

	em::raytracing raytracing;
	std::vector<em::acceleration_structure> blas;
	em::acceleration_structure tlas;

	VkViewport viewport;
	uint32_t current_frame;
	uint32_t last_frame;

	float min_depth;
	float max_depth;
};