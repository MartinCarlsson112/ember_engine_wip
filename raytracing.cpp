#include "raytracing.h"
#include "vulkan_utils.h"
#include "find_memory_type.h"
#include "vulkan_utils.h"

namespace em
{

	void raytracing::init_raytracing(const em::device& device)
	{
		properties = {};
		properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;

		VkPhysicalDeviceProperties2 device_properties{};

		device_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		device_properties.pNext = &properties;

		vkGetPhysicalDeviceProperties2(device.gpu, &device_properties);
		as_features = {};
		as_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
		as_features.accelerationStructure = VK_TRUE;

		VkPhysicalDeviceFeatures2 device_features{};
		device_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		device_features.pNext = &as_features;

		vkGetPhysicalDeviceFeatures2(device.gpu, &device_features);

		vkGetBufferDeviceAddressKHR = (PFN_vkGetBufferDeviceAddressKHR)(vkGetDeviceProcAddr(device.logical_device, "vkGetBufferDeviceAddressKHR"));
		vkCmdBuildAccelerationStructuresKHR = (PFN_vkCmdBuildAccelerationStructuresKHR)(vkGetDeviceProcAddr(device.logical_device, "vkCmdBuildAccelerationStructuresKHR"));
		vkBuildAccelerationStructuresKHR = (PFN_vkBuildAccelerationStructuresKHR)(vkGetDeviceProcAddr(device.logical_device, "vkBuildAccelerationStructuresKHR"));
		vkCreateAccelerationStructureKHR = (PFN_vkCreateAccelerationStructureKHR)(vkGetDeviceProcAddr(device.logical_device, "vkCreateAccelerationStructureKHR"));
		vkDestroyAccelerationStructureKHR = (PFN_vkDestroyAccelerationStructureKHR)(vkGetDeviceProcAddr(device.logical_device, "vkDestroyAccelerationStructureKHR"));
		vkGetAccelerationStructureBuildSizesKHR = (PFN_vkGetAccelerationStructureBuildSizesKHR)(vkGetDeviceProcAddr(device.logical_device, "vkGetAccelerationStructureBuildSizesKHR"));
		vkGetAccelerationStructureDeviceAddressKHR = (PFN_vkGetAccelerationStructureDeviceAddressKHR)(vkGetDeviceProcAddr(device.logical_device, "vkGetAccelerationStructureDeviceAddressKHR"));
		vkCmdTraceRaysKHR = (PFN_vkCmdTraceRaysKHR)(vkGetDeviceProcAddr(device.logical_device, "vkCmdTraceRaysKHR"));
		vkGetRayTracingShaderGroupHandlesKHR = (PFN_vkGetRayTracingShaderGroupHandlesKHR)(vkGetDeviceProcAddr(device.logical_device, "vkGetRayTracingShaderGroupHandlesKHR"));
		vkCreateRayTracingPipelinesKHR = (PFN_vkCreateRayTracingPipelinesKHR)(vkGetDeviceProcAddr(device.logical_device, "vkCreateRayTracingPipelinesKHR"));
	}

	void raytracing::create_acceleration_structure(const em::device& device, acceleration_structure& as, VkAccelerationStructureTypeKHR type, VkAccelerationStructureBuildSizesInfoKHR build_size_info)
	{
		if (as.buffer.buffer != VK_NULL_HANDLE)
		{
			device.destroy_buffer(as.buffer);
			as.address = VK_NULL_HANDLE;
			as.handle = VK_NULL_HANDLE;
		}

		device.create_buffer(as.buffer,
			build_size_info.accelerationStructureSize,
			VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR);

		VkAccelerationStructureCreateInfoKHR as_create_info{};

		as_create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
		as_create_info.buffer = as.buffer.buffer;
		as_create_info.size = build_size_info.accelerationStructureSize;
		as_create_info.type = type;

		VK_CHECK_RESULT(vkCreateAccelerationStructureKHR(device.logical_device, &as_create_info, nullptr, &as.handle))

			VkAccelerationStructureDeviceAddressInfoKHR as_device_address_info {};
		as_device_address_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
		as_device_address_info.accelerationStructure = as.handle;
		as.address = vkGetAccelerationStructureDeviceAddressKHR(device.logical_device, &as_device_address_info);
	}

	void raytracing::delete_scratch_buffer(const em::device& device, scratch_buffer& buffer)
	{
		device.destroy_buffer(buffer.buffer);
	}

	uint64_t raytracing::get_buffer_device_address(const em::device& device, VkBuffer buffer) const
	{
		VkBufferDeviceAddressInfoKHR buffer_device_address_info{};
		buffer_device_address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		buffer_device_address_info.buffer = buffer;
		return vkGetBufferDeviceAddressKHR(device.logical_device, &buffer_device_address_info);
	}

	void raytracing::create_storage_image(const em::device& device, VkFormat format, VkExtent3D extent)
	{
		if (storage.image != VK_NULL_HANDLE)
		{
			vkDestroyImageView(device.logical_device, storage.imageview, nullptr);
			vkDestroyImage(device.logical_device, storage.image, nullptr);
			vkFreeMemory(device.logical_device, storage.image_memory, nullptr);
			storage = storage_image();

		}

		width = extent.width;
		height = extent.height;


		VkImageCreateInfo image_create_info{};

		image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_create_info.format = format;
		image_create_info.extent = extent;
		image_create_info.mipLevels = 1;
		image_create_info.arrayLayers = 1;
		image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		image_create_info.imageType = VK_IMAGE_TYPE_2D;

		image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
		image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VK_CHECK_RESULT(vkCreateImage(device.logical_device, &image_create_info, nullptr, &storage.image))

			VkMemoryRequirements memory_requirements {};

		vkGetImageMemoryRequirements(device.logical_device, storage.image, &memory_requirements);

		VkMemoryAllocateInfo memory_alloc_info{};
		memory_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memory_alloc_info.memoryTypeIndex = find_memory_type(device.gpu, memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		memory_alloc_info.allocationSize = memory_requirements.size;


		VK_CHECK_RESULT(vkAllocateMemory(device.logical_device, &memory_alloc_info, nullptr, &storage.image_memory))
			VK_CHECK_RESULT(vkBindImageMemory(device.logical_device, storage.image, storage.image_memory, 0))

			VkImageSubresourceRange image_subresource_range {};
		image_subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_subresource_range.levelCount = 1;
		image_subresource_range.layerCount = 1;
		image_subresource_range.baseMipLevel = 0;
		image_subresource_range.baseArrayLayer = 0;

		VkImageViewCreateInfo image_view_create_info{};
		image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_create_info.format = format;
		image_view_create_info.image = storage.image;
		image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_create_info.subresourceRange = image_subresource_range;


		VK_CHECK_RESULT(vkCreateImageView(device.logical_device, &image_view_create_info, nullptr, &storage.imageview))

		VkCommandBuffer cmd_buffer = device.create_command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		device.begin_command_buffer(cmd_buffer);
		VkImageMemoryBarrier image_memory_barrier{};
		image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
		image_memory_barrier.image = storage.image;
		image_memory_barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		image_memory_barrier.srcAccessMask = 0;

		VkPipelineStageFlags src_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		VkPipelineStageFlags dst_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

		vkCmdPipelineBarrier(cmd_buffer, src_stage_mask, dst_stage_mask, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);

		device.flush_command_buffer(cmd_buffer, device.graphics_queue);
	}


	void raytracing::delete_storage_image(const em::device& device)
	{
		vkDestroyImageView(device.logical_device, storage.imageview, nullptr);
		vkDestroyImage(device.logical_device, storage.image, nullptr);
		vkFreeMemory(device.logical_device, storage.image_memory, nullptr);

		storage.imageview = VK_NULL_HANDLE;
		storage.image = VK_NULL_HANDLE;
		storage.image_memory = VK_NULL_HANDLE;
	}

	void raytracing::create_shader_binding_table(const em::device& device, shader_binding_table& sbt, uint32_t count)
	{
		device.create_buffer(sbt.bo, sbt.size, sbt.usage, sbt.memory_properties);
		sbt.strided_device_address_region = get_sbt_entry_strided_device_address_region(device, sbt.bo.buffer, count);
		VK_CHECK_RESULT(vkMapMemory(device.logical_device, sbt.bo.buffer_memory, 0, VK_WHOLE_SIZE, 0, &sbt.data))
	}

	void raytracing::create_scratch_buffer(const em::device& device, VkDeviceSize size, scratch_buffer& buffer)
	{
		device.create_buffer(buffer.buffer, size,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR);
		buffer.address = get_buffer_device_address(device, buffer.buffer.buffer);
	}


	void raytracing::delete_acceleration_structure(const em::device& device, acceleration_structure& as)
	{
		device.destroy_buffer(as.buffer);
		vkDestroyAccelerationStructureKHR(device.logical_device, as.handle, nullptr);
	}

	VkStridedDeviceAddressRegionKHR raytracing::get_sbt_entry_strided_device_address_region(const em::device& device, VkBuffer buffer, uint32_t handle_count)
	{
		const uint32_t handle_size_aligned = (properties.shaderGroupHandleSize + properties.shaderGroupHandleAlignment - 1) & ~(properties.shaderGroupHandleAlignment - 1);
		VkStridedDeviceAddressRegionKHR stridedDeviceAddressRegionKHR{};
		stridedDeviceAddressRegionKHR.deviceAddress = get_buffer_device_address(device, buffer);
		stridedDeviceAddressRegionKHR.stride = handle_size_aligned;
		stridedDeviceAddressRegionKHR.size = (uint64_t)handle_size_aligned * handle_count;
		return stridedDeviceAddressRegionKHR;
	}


	void raytracing::create_blas(const em::device& device, std::vector<acceleration_structure>& blas, const std::vector<mesh_batch>& batches)
	{
		for (int i = 0; i < batches.size(); i++)
		{
			VkDeviceOrHostAddressConstKHR vb_device_address;
			vb_device_address.deviceAddress = get_buffer_device_address(device, batches[i].vbo);
			VkAccelerationStructureGeometryKHR acceleration_structure_geometry{ };
			acceleration_structure_geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
			acceleration_structure_geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
			acceleration_structure_geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
			acceleration_structure_geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
			acceleration_structure_geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
			acceleration_structure_geometry.geometry.triangles.vertexData = vb_device_address;
			acceleration_structure_geometry.geometry.triangles.maxVertex = batches[i].vertex_count;
			acceleration_structure_geometry.geometry.triangles.vertexStride = sizeof(vertex);
			acceleration_structure_geometry.geometry.triangles.indexType = VK_INDEX_TYPE_NONE_KHR;
			acceleration_structure_geometry.geometry.triangles.transformData.deviceAddress = 0;
			acceleration_structure_geometry.geometry.triangles.transformData.hostAddress = nullptr;


			VkAccelerationStructureBuildGeometryInfoKHR as_build_geo_info{  };
			as_build_geo_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
			as_build_geo_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
			as_build_geo_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
			as_build_geo_info.geometryCount = 1;
			as_build_geo_info.pGeometries = &acceleration_structure_geometry;

			const uint32_t number_triangles = batches[i].vertex_count / 3;
			VkAccelerationStructureBuildSizesInfoKHR as_build_sizes_info{};
			as_build_sizes_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
			vkGetAccelerationStructureBuildSizesKHR(
				device.logical_device,
				VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
				&as_build_geo_info,
				&number_triangles,
				&as_build_sizes_info);

			blas.push_back(acceleration_structure{});

			create_acceleration_structure(device, blas[i], VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR, as_build_sizes_info);

			em::scratch_buffer sb;
			create_scratch_buffer(device, as_build_sizes_info.buildScratchSize, sb);

			VkAccelerationStructureBuildGeometryInfoKHR sb_as_build_geo_info{};
			sb_as_build_geo_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
			sb_as_build_geo_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
			sb_as_build_geo_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
			sb_as_build_geo_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
			sb_as_build_geo_info.dstAccelerationStructure = blas[i].handle;
			sb_as_build_geo_info.geometryCount = 1;
			sb_as_build_geo_info.pGeometries = &acceleration_structure_geometry;
			sb_as_build_geo_info.scratchData.deviceAddress = sb.address;

			VkAccelerationStructureBuildRangeInfoKHR as_build_range_info{};
			as_build_range_info.primitiveCount = number_triangles;
			as_build_range_info.primitiveOffset = 0;
			as_build_range_info.firstVertex = 0;
			as_build_range_info.transformOffset = 0;

			std::vector<VkAccelerationStructureBuildRangeInfoKHR*> as_build_range_info_array = { &as_build_range_info };

			if (as_features.accelerationStructureHostCommands)
			{
				VK_CHECK_RESULT(vkBuildAccelerationStructuresKHR(device.logical_device, VK_NULL_HANDLE, 1, &sb_as_build_geo_info, as_build_range_info_array.data()))
			}
			else
			{
				auto cmd_buffer2 = device.create_command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
				device.begin_command_buffer(cmd_buffer2);
				vkCmdBuildAccelerationStructuresKHR(cmd_buffer2, 1, &sb_as_build_geo_info, as_build_range_info_array.data());
				VkMemoryBarrier barrier{ VK_STRUCTURE_TYPE_MEMORY_BARRIER };
				barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
				barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
				vkCmdPipelineBarrier(cmd_buffer2,
					VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
					VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
					0, 1, &barrier, 0, nullptr, 0, nullptr);

				device.flush_command_buffer(cmd_buffer2, device.graphics_queue);
			}

			VkAccelerationStructureDeviceAddressInfoKHR as_device_address_info{ };
			as_device_address_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
			as_device_address_info.accelerationStructure = blas[i].handle;
			blas[i].address = vkGetAccelerationStructureDeviceAddressKHR(device.logical_device, &as_device_address_info);

			delete_scratch_buffer(device, sb);
		}

	
	}



	void raytracing::create_tlas(const em::device& device, const std::vector<acceleration_structure>& blas, acceleration_structure& tlas, const std::vector <mesh_batch>& batches)
	{
		VkCommandBuffer cmd_buffer = device.create_command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		device.begin_command_buffer(cmd_buffer);

		uint32_t total_size = 0;
		for (int i = 0; i < batches.size(); i++)
		{
			total_size += batches[i].count;
		}

		std::vector<VkAccelerationStructureInstanceKHR> instances(total_size);

		uint32_t counter = 0;
		for (int i = 0; i < batches.size(); i++)
		{
			for (int j = 0; j < batches[i].count; j++)
			{

				auto& model_matrix = batches[i].model[j];

				VkTransformMatrixKHR transform_matrix = { 
					model_matrix[0], model_matrix[4], model_matrix[8], model_matrix[12],
					model_matrix[1], model_matrix[5], model_matrix[9], model_matrix[13],
					model_matrix[2], model_matrix[6], model_matrix[10], model_matrix[14],
				};

				VkAccelerationStructureInstanceKHR instance{ };
				instance.transform = transform_matrix;
				instance.instanceCustomIndex = 0;
				instance.mask = 0XFF;
				instance.instanceShaderBindingTableRecordOffset = 0;
				instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
				instance.accelerationStructureReference = blas[i].address;
				instances[counter] = instance;
				counter++;
			}
		}

		VkDeviceSize instanceDescsSizeInBytes = instances.size() * sizeof(VkAccelerationStructureInstanceKHR);
		buffer_object instance_buffer;
		device.create_buffer(instance_buffer, instanceDescsSizeInBytes, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT);
		void* data;
		VK_CHECK_RESULT(vkMapMemory(device.logical_device, instance_buffer.buffer_memory, 0, instanceDescsSizeInBytes, 0, &data))
		memcpy(data, instances.data(), (size_t)instanceDescsSizeInBytes);
		vkUnmapMemory(device.logical_device, instance_buffer.buffer_memory);

		VkBufferDeviceAddressInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
		bufferInfo.buffer = instance_buffer.buffer;
		VkDeviceAddress instanceAddress = vkGetBufferDeviceAddress(device.logical_device, &bufferInfo);

		// Make sure the copy of the instance buffer are copied before triggering the
		// acceleration structure build
		VkMemoryBarrier barrier{ VK_STRUCTURE_TYPE_MEMORY_BARRIER };
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
		vkCmdPipelineBarrier(cmd_buffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
			0, 1, &barrier, 0, nullptr, 0, nullptr);


		VkAccelerationStructureGeometryInstancesDataKHR instances_vk{
		 VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR };
		instances_vk.arrayOfPointers = VK_FALSE;
		instances_vk.data.deviceAddress = instanceAddress;


		VkAccelerationStructureGeometryKHR as_geometry{ };
		as_geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		as_geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		as_geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		as_geometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		as_geometry.geometry.instances.arrayOfPointers = VK_FALSE;
		as_geometry.geometry.instances = instances_vk;


		VkAccelerationStructureBuildGeometryInfoKHR as_build_geo_info{};
		as_build_geo_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		as_build_geo_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		as_build_geo_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		as_build_geo_info.geometryCount = 1;
		as_build_geo_info.pGeometries = &as_geometry;

		uint32_t primitive_count = (uint32_t)instances.size();

		VkAccelerationStructureBuildSizesInfoKHR as_build_sizes_info{};
		as_build_sizes_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

		vkGetAccelerationStructureBuildSizesKHR(device.logical_device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
			&as_build_geo_info, &primitive_count, &as_build_sizes_info);

		create_acceleration_structure(device, tlas, VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, as_build_sizes_info);

		em::scratch_buffer sb;
		create_scratch_buffer(device, as_build_sizes_info.buildScratchSize, sb);


		VkAccelerationStructureBuildGeometryInfoKHR as_build_geo_info_2{};
		as_build_geo_info_2.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		as_build_geo_info_2.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		as_build_geo_info_2.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		as_build_geo_info_2.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		as_build_geo_info_2.dstAccelerationStructure = tlas.handle;
		as_build_geo_info_2.geometryCount = 1;
		as_build_geo_info_2.pGeometries = &as_geometry;
		as_build_geo_info_2.scratchData.deviceAddress = sb.address;


		VkAccelerationStructureBuildRangeInfoKHR as_build_range_info{ static_cast<uint32_t>(instances.size()), 0, 0, 0};
		const VkAccelerationStructureBuildRangeInfoKHR* as_build_range_info_array = &as_build_range_info;

		vkCmdBuildAccelerationStructuresKHR(cmd_buffer, 1, &as_build_geo_info_2, &as_build_range_info_array);
		device.flush_command_buffer(cmd_buffer, device.graphics_queue);


		VkAccelerationStructureDeviceAddressInfoKHR as_device_address_info{};
		as_device_address_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
		as_device_address_info.accelerationStructure = tlas.handle;
		tlas.address = vkGetAccelerationStructureDeviceAddressKHR(device.logical_device, &as_device_address_info);

		delete_scratch_buffer(device, sb);
	}




	void raytracing::create_pipeline(const em::device& device)
	{
		VkDescriptorSetLayoutBinding as_layout_binding{};
		as_layout_binding.binding = 0;
		as_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		as_layout_binding.descriptorCount = 1;
		as_layout_binding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

		VkDescriptorSetLayoutBinding result_image_layout_binding{};
		result_image_layout_binding.binding = 1;
		result_image_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		result_image_layout_binding.descriptorCount = 1;
		result_image_layout_binding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

		VkDescriptorSetLayoutBinding uniform_buffer_layout_binding{};
		uniform_buffer_layout_binding.binding = 2;
		uniform_buffer_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniform_buffer_layout_binding.descriptorCount = 1;
		uniform_buffer_layout_binding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

		VkDescriptorSetLayoutBinding lbo_layout_binding{};
		lbo_layout_binding.binding = 3;
		lbo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		lbo_layout_binding.descriptorCount = 1;
		lbo_layout_binding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;


		VkDescriptorSetLayoutBinding vbo_layout_binding{};
		vbo_layout_binding.binding = 4;
		vbo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		vbo_layout_binding.descriptorCount = 1;
		vbo_layout_binding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

		std::vector<VkDescriptorSetLayoutBinding> bindings{ as_layout_binding, result_image_layout_binding, uniform_buffer_layout_binding, vbo_layout_binding, lbo_layout_binding };

		VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{};
		descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptor_set_layout_create_info.bindingCount = static_cast<uint32_t>(bindings.size());
		descriptor_set_layout_create_info.pBindings = bindings.data();

		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device.logical_device, &descriptor_set_layout_create_info, nullptr, &descriptor_set_layout));

		VkPipelineLayoutCreateInfo pipeline_create_info{};
		pipeline_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_create_info.setLayoutCount = 1;
		pipeline_create_info.pSetLayouts = &descriptor_set_layout;

		//VkPushConstantRange push_constant_range = VkPushConstantRange();
		//push_constant_range.size = sizeof(float4x4);
		//push_constant_range.offset = 0;
		//push_constant_range.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

		//pipeline_create_info.pPushConstantRanges = &push_constant_range;
		//pipeline_create_info.pushConstantRangeCount = 1;

		VK_CHECK_RESULT(vkCreatePipelineLayout(device.logical_device, &pipeline_create_info, nullptr, &pipeline_layout));

		std::vector<VkPipelineShaderStageCreateInfo> shader_stages;

		shaders.push_back(load_shader_impl(device, "shader/basic.rgen.spv", VK_SHADER_STAGE_RAYGEN_BIT_KHR));


		shader_stages.push_back(VkPipelineShaderStageCreateInfo{});
		shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[0].module = shaders[shaders.size() - 1].shader_module;
		shader_stages[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		shader_stages[0].pName = "main";

		VkRayTracingShaderGroupCreateInfoKHR shader_group_create_info_raygen{};
		shader_group_create_info_raygen.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		shader_group_create_info_raygen.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		shader_group_create_info_raygen.generalShader = static_cast<uint32_t>(shader_stages.size() - 1);
		shader_group_create_info_raygen.closestHitShader = VK_SHADER_UNUSED_KHR;
		shader_group_create_info_raygen.anyHitShader = VK_SHADER_UNUSED_KHR;
		shader_group_create_info_raygen.intersectionShader = VK_SHADER_UNUSED_KHR;
		shader_groups.push_back(shader_group_create_info_raygen);

		shaders.push_back(load_shader_impl(device, "shader/basic.rmiss.spv", VK_SHADER_STAGE_MISS_BIT_KHR));
		shader_stages.push_back(VkPipelineShaderStageCreateInfo{});
		shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[1].module = shaders[shaders.size() - 1].shader_module;
		shader_stages[1].stage = VK_SHADER_STAGE_MISS_BIT_KHR;
		shader_stages[1].pName = "main";

		VkRayTracingShaderGroupCreateInfoKHR shader_group_create_info_miss{};
		shader_group_create_info_miss.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		shader_group_create_info_miss.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		shader_group_create_info_miss.generalShader = static_cast<uint32_t>(shader_stages.size() - 1);
		shader_group_create_info_miss.closestHitShader = VK_SHADER_UNUSED_KHR;
		shader_group_create_info_miss.anyHitShader = VK_SHADER_UNUSED_KHR;
		shader_group_create_info_miss.intersectionShader = VK_SHADER_UNUSED_KHR;
		shader_groups.push_back(shader_group_create_info_miss);


		shaders.push_back(load_shader_impl(device, "shader/shadows.rmiss.spv", VK_SHADER_STAGE_MISS_BIT_KHR));

		shader_stages.push_back(VkPipelineShaderStageCreateInfo{});
		shader_stages[2].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[2].module = shaders[shaders.size() - 1].shader_module;
		shader_stages[2].stage = VK_SHADER_STAGE_MISS_BIT_KHR;
		shader_stages[2].pName = "main";

		VkRayTracingShaderGroupCreateInfoKHR shader_group_create_info_shadow_miss{};
		shader_group_create_info_shadow_miss.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		shader_group_create_info_shadow_miss.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		shader_group_create_info_shadow_miss.generalShader = static_cast<uint32_t>(shader_stages.size() - 1);
		shader_group_create_info_shadow_miss.closestHitShader = VK_SHADER_UNUSED_KHR;
		shader_group_create_info_shadow_miss.anyHitShader = VK_SHADER_UNUSED_KHR;
		shader_group_create_info_shadow_miss.intersectionShader = VK_SHADER_UNUSED_KHR;
		shader_groups.push_back(shader_group_create_info_shadow_miss);

		shaders.push_back(load_shader_impl(device, "shader/basic.rchit.spv", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR));
		shader_stages.push_back(VkPipelineShaderStageCreateInfo{});
		shader_stages[3].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[3].module = shaders[shaders.size() - 1].shader_module;
		shader_stages[3].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		shader_stages[3].pName = "main";

		VkRayTracingShaderGroupCreateInfoKHR shader_group_create_info_closest_hit{};
		shader_group_create_info_closest_hit.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		shader_group_create_info_closest_hit.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
		shader_group_create_info_closest_hit.generalShader = VK_SHADER_UNUSED_KHR;
		shader_group_create_info_closest_hit.closestHitShader = static_cast<uint32_t>(shader_stages.size() - 1);
		shader_group_create_info_closest_hit.anyHitShader = VK_SHADER_UNUSED_KHR;
		shader_group_create_info_closest_hit.intersectionShader = VK_SHADER_UNUSED_KHR;
		shader_groups.push_back(shader_group_create_info_closest_hit);

		VkRayTracingPipelineCreateInfoKHR rt_pipeline_create_info{};
		rt_pipeline_create_info.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
		rt_pipeline_create_info.stageCount = static_cast<uint32_t>(shader_stages.size());
		rt_pipeline_create_info.pStages = shader_stages.data();
		rt_pipeline_create_info.groupCount = static_cast<uint32_t>(shader_groups.size());
		rt_pipeline_create_info.pGroups = shader_groups.data();
		rt_pipeline_create_info.maxPipelineRayRecursionDepth = 2;
		rt_pipeline_create_info.layout = pipeline_layout;
		VK_CHECK_RESULT(vkCreateRayTracingPipelinesKHR(device.logical_device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rt_pipeline_create_info, nullptr, &pipeline));
	}

	void raytracing::create_sbt(const em::device& device)
	{
		const uint32_t handle_size = properties.shaderGroupHandleSize;
		const uint32_t handle_size_aligned = (properties.shaderGroupHandleSize + properties.shaderGroupHandleAlignment - 1) & ~(properties.shaderGroupHandleAlignment - 1);;
		const uint32_t group_count = static_cast<uint32_t>(shader_groups.size());
		const uint32_t sbt_size = group_count * handle_size_aligned;

		std::vector<uint8_t> shader_handle_storage(sbt_size);
		VK_CHECK_RESULT(vkGetRayTracingShaderGroupHandlesKHR(device.logical_device, pipeline, 0, group_count, sbt_size, shader_handle_storage.data()));

		const VkBufferUsageFlags buffer_usage_flags = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		const VkMemoryPropertyFlags memory_usage_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		device.create_buffer(raygen_sbt.bo, handle_size_aligned, buffer_usage_flags, memory_usage_flags, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR);
		device.create_buffer(miss_sbt.bo, (uint64_t)handle_size_aligned * 2, buffer_usage_flags, memory_usage_flags, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR);
		device.create_buffer(closest_hit_sbt.bo, handle_size_aligned, buffer_usage_flags, memory_usage_flags, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR);

		void* raygen_data;
		void* miss_data;
		void* ch_data;

		raygen_sbt.strided_device_address_region = get_sbt_entry_strided_device_address_region(device, raygen_sbt.bo.buffer, 1);
		miss_sbt.strided_device_address_region = get_sbt_entry_strided_device_address_region(device, miss_sbt.bo.buffer, 2);
		closest_hit_sbt.strided_device_address_region = get_sbt_entry_strided_device_address_region(device, closest_hit_sbt.bo.buffer, 1);

		VK_CHECK_RESULT(vkMapMemory(device.logical_device, raygen_sbt.bo.buffer_memory, 0, handle_size_aligned, 0, &raygen_data))
			VK_CHECK_RESULT(vkMapMemory(device.logical_device, miss_sbt.bo.buffer_memory, 0, handle_size_aligned * 2, 0, &miss_data))
			VK_CHECK_RESULT(vkMapMemory(device.logical_device, closest_hit_sbt.bo.buffer_memory, 0, handle_size_aligned, 0, &ch_data))

			memcpy(raygen_data, shader_handle_storage.data(), handle_size);
		memcpy(miss_data, shader_handle_storage.data() + handle_size_aligned, handle_size * 2);
		memcpy(ch_data, shader_handle_storage.data() + handle_size_aligned * 3, handle_size);

		vkUnmapMemory(device.logical_device, raygen_sbt.bo.buffer_memory);
		vkUnmapMemory(device.logical_device, miss_sbt.bo.buffer_memory);
		vkUnmapMemory(device.logical_device, closest_hit_sbt.bo.buffer_memory);
	}

	void raytracing::create_descriptor_set_pools(const em::device& device, const std::vector<buffer_object>& ubos, const std::vector<buffer_object>& lbos)
	{
		std::vector<VkDescriptorPoolSize> pool_sizes = {
			{VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1},
			{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1},
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
			{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1},
		};

		std::vector<VkDescriptorSetLayout> descriptor_set_layouts(ubos.size(), descriptor_set_layout);

		descriptor_sets.resize(ubos.size());

		VkDescriptorPoolCreateInfo descriptor_pool_create_info{};
		descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptor_pool_create_info.pPoolSizes = pool_sizes.data();
		descriptor_pool_create_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
		descriptor_pool_create_info.maxSets = static_cast<uint32_t>(ubos.size());
		VK_CHECK_RESULT(vkCreateDescriptorPool(device.logical_device, &descriptor_pool_create_info, nullptr, &descriptor_pool));

		VkDescriptorSetAllocateInfo descriptor_set_allocate_info{};
		descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptor_set_allocate_info.descriptorPool = descriptor_pool;
		descriptor_set_allocate_info.pSetLayouts = descriptor_set_layouts.data();
		descriptor_set_allocate_info.descriptorSetCount = static_cast<uint32_t>(ubos.size());
		VK_CHECK_RESULT(vkAllocateDescriptorSets(device.logical_device, &descriptor_set_allocate_info, descriptor_sets.data()));
	}

	void raytracing::update_descriptor_sets(const em::device& device, const std::vector<mesh_batch>& batches, em::acceleration_structure& tlas, const std::vector<buffer_object>& ubos, const std::vector<buffer_object>& lbos)
	{
		VkWriteDescriptorSetAccelerationStructureKHR descriptor_as_info{};

		descriptor_as_info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
		descriptor_as_info.pNext = VK_NULL_HANDLE;
		descriptor_as_info.accelerationStructureCount = 1;
		descriptor_as_info.pAccelerationStructures = &tlas.handle;

		for (int i = 0; i < ubos.size(); i++)
		{
			VkWriteDescriptorSet as_write{};
			as_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			as_write.pNext = &descriptor_as_info;
			as_write.dstSet = descriptor_sets[i];
			as_write.dstBinding = 0;
			as_write.descriptorCount = 1;
			as_write.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

			VkDescriptorImageInfo storage_image_descriptor{ VK_NULL_HANDLE, storage.imageview, VK_IMAGE_LAYOUT_GENERAL };
			VkDescriptorBufferInfo vertex_buffer_descriptor{ batches[0].vbo, 0, VK_WHOLE_SIZE };
			VkDescriptorBufferInfo ubo_descriptor{ ubos[i].buffer, 0, VK_WHOLE_SIZE };
			VkDescriptorBufferInfo lbo_descriptor{ lbos[i].buffer, 0, VK_WHOLE_SIZE };

			VkWriteDescriptorSet storage_image_write{};
			storage_image_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			storage_image_write.dstSet = descriptor_sets[i];
			storage_image_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			storage_image_write.dstBinding = 1;
			storage_image_write.pImageInfo = &storage_image_descriptor;
			storage_image_write.descriptorCount = 1;

			VkWriteDescriptorSet ubo_write{};
			ubo_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			ubo_write.dstSet = descriptor_sets[i];
			ubo_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			ubo_write.dstBinding = 2;
			ubo_write.pBufferInfo = &ubo_descriptor;
			ubo_write.descriptorCount = 1;

			VkWriteDescriptorSet lbo_write{};
			lbo_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			lbo_write.dstSet = descriptor_sets[i];
			lbo_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			lbo_write.dstBinding = 3;
			lbo_write.pBufferInfo = &lbo_descriptor;
			lbo_write.descriptorCount = 1;

			VkWriteDescriptorSet vbo_write{};
			vbo_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			vbo_write.dstSet = descriptor_sets[i];
			vbo_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			vbo_write.dstBinding = 4;
			vbo_write.pBufferInfo = &vertex_buffer_descriptor;
			vbo_write.descriptorCount = 1;

			std::vector<VkWriteDescriptorSet> write_descriptor_sets = {
				as_write,
				storage_image_write,
				vbo_write,
				ubo_write,
				lbo_write };

			vkUpdateDescriptorSets(device.logical_device, static_cast<uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, VK_NULL_HANDLE);
		}
	}

	void raytracing::record_command_buffer(const em::device& device, uint32_t current_frame, VkCommandBuffer& cmd_buffer, const std::vector<mesh_batch>& batches, em::swapchain& swapchain, image_object shadow_map)
	{
		VkImageSubresourceRange sub_resource_range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

		vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);
		vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline_layout, 0, 1, &descriptor_sets[current_frame], 0, 0);

		VkStridedDeviceAddressRegionKHR empty_sbt_entry{};
		vkCmdTraceRaysKHR(cmd_buffer,
			&raygen_sbt.strided_device_address_region,
			&miss_sbt.strided_device_address_region,
			&closest_hit_sbt.strided_device_address_region,
			&empty_sbt_entry,
			width,
			height,
			1);

		VkImageMemoryBarrier current_map_image_memory_barrier{};
		current_map_image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		current_map_image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		current_map_image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		current_map_image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		current_map_image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		current_map_image_memory_barrier.image = shadow_map.image;
		current_map_image_memory_barrier.subresourceRange = sub_resource_range;
		current_map_image_memory_barrier.srcAccessMask = 0;
		current_map_image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		vkCmdPipelineBarrier(
			cmd_buffer,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &current_map_image_memory_barrier);

		VkImageMemoryBarrier tracing_output_image_memory_barrier{};
		tracing_output_image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		tracing_output_image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		tracing_output_image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		tracing_output_image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
		tracing_output_image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		tracing_output_image_memory_barrier.image = storage.image;
		tracing_output_image_memory_barrier.subresourceRange = sub_resource_range;
		tracing_output_image_memory_barrier.srcAccessMask = 0;
		tracing_output_image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;


		//copy ray trace output to image
		vkCmdPipelineBarrier(
			cmd_buffer,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &tracing_output_image_memory_barrier);

		//copy region
		VkImageCopy copy_region{};
		copy_region.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		copy_region.srcOffset = { 0,0,0 };
		copy_region.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		copy_region.dstOffset = { 0,0,0 };
		copy_region.extent = { width, height, 1 };
		vkCmdCopyImage(cmd_buffer,
			storage.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			shadow_map.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &copy_region);


		//transition swapchain back for presentation
		VkImageMemoryBarrier swapchain_back_for_pres_image_memory_barrier{};
		swapchain_back_for_pres_image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		swapchain_back_for_pres_image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		swapchain_back_for_pres_image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		swapchain_back_for_pres_image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		swapchain_back_for_pres_image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		swapchain_back_for_pres_image_memory_barrier.image = shadow_map.image;
		swapchain_back_for_pres_image_memory_barrier.subresourceRange = sub_resource_range;
		swapchain_back_for_pres_image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		swapchain_back_for_pres_image_memory_barrier.dstAccessMask = 0;

		vkCmdPipelineBarrier(
			cmd_buffer,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &swapchain_back_for_pres_image_memory_barrier);

		//transition ray tracing output image back to general layout
		VkImageMemoryBarrier tracing_output_to_general_image_memory_barrier{};
		tracing_output_to_general_image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		tracing_output_to_general_image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		tracing_output_to_general_image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		tracing_output_to_general_image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		tracing_output_to_general_image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
		tracing_output_to_general_image_memory_barrier.image = storage.image;
		tracing_output_to_general_image_memory_barrier.subresourceRange = sub_resource_range;
		tracing_output_to_general_image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		tracing_output_to_general_image_memory_barrier.dstAccessMask = 0;


		//copy raytrace output to image
		vkCmdPipelineBarrier(
			cmd_buffer,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &tracing_output_to_general_image_memory_barrier);
	}

}