#pragma once
#include "vulkan\vulkan.h"
#include "queue_family_indices.h"
#include "swapchain_support_details.h"
#include "vertex.h"
#include "image_object.h"
#include "mesh.h"
#include "skinned_vertex.h"

namespace em
{

	struct device
	{
		device() :logical_device(VK_NULL_HANDLE), gpu(VK_NULL_HANDLE), present_queue(VK_NULL_HANDLE), graphics_queue(VK_NULL_HANDLE), pool(VK_NULL_HANDLE){}

		void create_buffer(buffer_object& bo, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)const;
		void create_buffer(buffer_object& bo, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkMemoryAllocateFlags alloc_flags)const;
		void copy_buffer(VkBuffer dst_buffer, VkBuffer src_buffer, VkDeviceSize size, VkDeviceSize src_offset = 0, VkDeviceSize dst_offset = 0) const;
		void create_image(int width, int height, em::image_object& io, const image_options& options) const;

		void load_mesh(const std::vector<skinned_vertex>& verts, mesh& _mesh) const;
		void load_mesh(const std::vector<vertex>& verts, mesh& _mesh) const;

		void load_image(char* data, int width, int height, em::image_object& io, const image_options& options) const;
		void load_texture(char* data, int width, int height, em::image_object& io, const image_options& options) const;

		void destroy_mesh(mesh _mesh) const;
		void destroy_buffer(buffer_object& bo) const;
		void destroy_image(em::image_object& io)const;



		VkDevice logical_device;
		VkPhysicalDevice gpu;
		VkQueue present_queue;
		VkQueue graphics_queue;
		swapchain_support_details swapchain_support;
		queue_family_indices indices;
		VkCommandPool pool;

		bool create(const VkInstance& instance, const VkSurfaceKHR& surface);
		void create_command_pool();
		void copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) const;

		VkCommandBuffer create_command_buffer(VkCommandBufferLevel level, uint32_t buffer_count = 1)const;
		void begin_command_buffer(VkCommandBuffer cmd_buffer)const;
		void flush_command_buffer(VkCommandBuffer cmd_buffer, VkQueue queue, bool free = true)const;
		void transition_image_layout(VkImageLayout old_layout, VkImageLayout new_layout, VkImage image) const;

	private:
		void init_buffers(const std::vector<vertex>& verts, mesh& _mesh) const;
		void init_buffers(const std::vector<skinned_vertex>& verts, mesh& _mesh) const;
	protected:

		queue_family_indices find_queue_families(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);
		bool pick_physical_device(const VkInstance& instance, const VkSurfaceKHR& surface);
		void pick_logical_device(const VkSurfaceKHR& surface);
		bool is_device_suitable(const VkPhysicalDevice& device, const VkSurfaceKHR& surface, queue_family_indices* outIndices, swapchain_support_details* details);
		bool check_device_extension_support(VkPhysicalDevice device);
	};
}
