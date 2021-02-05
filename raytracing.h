#pragma once
#include "vulkan/vulkan.h"
#include "device.h"
#include "storage_image.h"
#include "mesh_batch.h"
#include "buffer_object.h"
#include "shader.h"
#include "swapchain.h"
#include "image_object.h"
namespace em
{
	struct acceleration_structure
	{
		VkAccelerationStructureKHR handle;
		uint64_t address;
		buffer_object buffer;
	};

	struct scratch_buffer
	{
		uint64_t address;
		buffer_object buffer;
	};


	struct shader_binding_table
	{
		VkStridedDeviceAddressRegionKHR strided_device_address_region{};
		buffer_object bo;
		VkDeviceSize size;
		VkDeviceSize alignment;
		VkDescriptorBufferInfo descriptor;
		VkBufferUsageFlags usage;
		VkMemoryPropertyFlags memory_properties;

		void* data = nullptr;
	};

	struct raytracing
	{

		PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR;
		PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR;
		PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR;
		PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR;
		PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR;
		PFN_vkBuildAccelerationStructuresKHR vkBuildAccelerationStructuresKHR;
		PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR;
		PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR;
		PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR;
		PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR;

		void init_raytracing(const em::device& device);

		VkPhysicalDeviceRayTracingPipelinePropertiesKHR properties{};
		VkPhysicalDeviceAccelerationStructureFeaturesKHR as_features{};

		void create_acceleration_structure(const em::device& device, acceleration_structure& as, VkAccelerationStructureTypeKHR type, VkAccelerationStructureBuildSizesInfoKHR build_size_info);
		void delete_acceleration_structure(const em::device& device, acceleration_structure& as);

		void create_scratch_buffer(const em::device& device, VkDeviceSize size, scratch_buffer& buffer);
		void delete_scratch_buffer(const em::device& device, scratch_buffer& buffer);

		uint64_t get_buffer_device_address(const em::device& device, VkBuffer buffer) const;

		uint32_t width, height;
		storage_image storage;
		void create_storage_image(const em::device& device, VkFormat format, VkExtent3D extent);
		void delete_storage_image(const em::device& device);

		void create_shader_binding_table(const em::device& device, shader_binding_table& sbt, uint32_t count);
		VkStridedDeviceAddressRegionKHR get_sbt_entry_strided_device_address_region(const em::device& device, VkBuffer buffer, uint32_t handle_count);

		void create_blas(const em::device& device, std::vector<acceleration_structure>& blas, const std::vector<mesh_batch>& batches);
		void create_tlas(const em::device& device, 
			const std::vector<acceleration_structure>& blas, 
			acceleration_structure& tlas, 
			const std::vector <mesh_batch>& batches);
		void create_pipeline(const em::device& device);
		void create_sbt(const em::device& device);

		VkDescriptorSetLayout descriptor_set_layout;
		VkDescriptorPool descriptor_pool;
		std::vector<VkDescriptorSet> descriptor_sets;


		void create_descriptor_set_pools(const em::device& device, const std::vector<buffer_object>& ubos, const std::vector<buffer_object>& lbos);

		void update_descriptor_sets(
			const em::device& device, 
			const std::vector<mesh_batch>& batches,
			em::acceleration_structure& tlas, 
			const std::vector<buffer_object>& ubos, 
			const std::vector<buffer_object>& lbos);

		void record_command_buffer(const em::device& device, uint32_t current_frame, VkCommandBuffer& cmd_buffer, const std::vector<mesh_batch>& batches, em::swapchain& swapchain, image_object shadow_map);

		VkPipelineLayout pipeline_layout;
		VkPipeline pipeline;

		shader_binding_table raygen_sbt;
		shader_binding_table miss_sbt;
		shader_binding_table closest_hit_sbt;

		std::vector<VkRayTracingShaderGroupCreateInfoKHR> shader_groups;
		std::vector<shader> shaders;

	};
}