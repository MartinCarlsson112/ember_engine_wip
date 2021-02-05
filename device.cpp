#include "device.h"
#include "ofbx.h"
#include "find_memory_type.h"
#include "vulkan_utils.h"

#include "vk_extensions.h"
#include "buffer_object.h"
namespace em
{
	void device::init_buffers(const std::vector<vertex>& verts, mesh& _mesh) const
	{
		buffer_object staging_buffer;
		VkDeviceSize buffer_size = sizeof(verts[0]) * verts.size();
		create_buffer(staging_buffer, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		void* data;
		VK_CHECK_RESULT(vkMapMemory(logical_device, staging_buffer.buffer_memory, 0, buffer_size, 0, &data))
		memcpy(data, verts.data(), (size_t)buffer_size);
		vkUnmapMemory(logical_device, staging_buffer.buffer_memory);
		create_buffer(_mesh.vbo, buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR);
		copy_buffer(_mesh.vbo.buffer, staging_buffer.buffer, buffer_size);
		destroy_buffer(staging_buffer);
		_mesh.vertex_count = (uint32_t)verts.size();
	}

	void device::load_mesh(const std::vector<vertex>& verts, mesh& _mesh) const
	{
		init_buffers(verts, _mesh);
	}

	void device::load_image(char* data, int width, int height, image_object& io, const image_options& options) const
	{
		buffer_object temp_buffer;
		VkDeviceSize imageSize = width * height * 4;
		create_buffer(temp_buffer, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		void* temp_data;
		vkMapMemory(logical_device, temp_buffer.buffer_memory, 0, imageSize, 0, &temp_data);
		memcpy(temp_data, data, static_cast<size_t>(imageSize));
		vkUnmapMemory(logical_device, temp_buffer.buffer_memory);

		create_image(width, height, io, options);

		transition_image_layout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, io.image);
		copy_buffer_to_image(temp_buffer.buffer, io.image, static_cast<uint32_t>(width), static_cast<uint32_t>(height));

		transition_image_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, io.image);
		destroy_buffer(temp_buffer);
	}

	void device::load_texture(char* data, int width, int height, image_object& io, const image_options& options) const
	{

	}

	void device::destroy_mesh(mesh _mesh) const
	{
		destroy_buffer(_mesh.vbo);
	}

	void device::copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) const
	{
		VkCommandBuffer cmd_buffer = create_command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
		begin_command_buffer(cmd_buffer);

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			width,
			height,
			1
		};
		vkCmdCopyBufferToImage(cmd_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		flush_command_buffer(cmd_buffer, graphics_queue);
	}

	VkCommandBuffer device::create_command_buffer(VkCommandBufferLevel level, uint32_t buffer_count /*= 1*/)const
	{
		VkCommandBufferAllocateInfo cmd_buffer_alloc_info{};
		cmd_buffer_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmd_buffer_alloc_info.commandPool = pool;
		cmd_buffer_alloc_info.level = level;
		cmd_buffer_alloc_info.commandBufferCount = buffer_count;
		VkCommandBuffer cmd_buffer;
		VK_CHECK_RESULT(vkAllocateCommandBuffers(logical_device, &cmd_buffer_alloc_info, &cmd_buffer));

		return cmd_buffer;
	}

	void device::begin_command_buffer(VkCommandBuffer cmd_buffer)const
	{
		VkCommandBufferBeginInfo cmd_buffer_begin_info{};
		cmd_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmd_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		VK_CHECK_RESULT(vkBeginCommandBuffer(cmd_buffer, &cmd_buffer_begin_info));
	}

	void device::flush_command_buffer(VkCommandBuffer cmd_buffer, VkQueue queue, bool free /*= true*/)const
	{
		VK_CHECK_RESULT(vkEndCommandBuffer(cmd_buffer))
		VkSubmitInfo submit_info {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;

		submit_info.pCommandBuffers = &cmd_buffer;

		VkFenceCreateInfo fence_create_info {};
		fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_create_info.flags = 0;

		VkFence fence;

		VK_CHECK_RESULT(vkCreateFence(logical_device, &fence_create_info, nullptr, &fence))

		VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submit_info, fence))

		VK_CHECK_RESULT(vkWaitForFences(logical_device, 1, &fence, VK_TRUE, UINT64_MAX))

		vkDestroyFence(logical_device, fence, nullptr);

		if (free)
		{
			vkFreeCommandBuffers(logical_device, pool, 1, &cmd_buffer);
		}
	}

	void device::transition_image_layout(VkImageLayout old_layout, VkImageLayout new_layout, VkImage image) const
	{
		VkCommandBuffer command_buffer = create_command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
		begin_command_buffer(command_buffer);

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = old_layout;
		barrier.newLayout = new_layout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags source_stage;
		VkPipelineStageFlags destination_stage;

		if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else {
			std::cout << "unsupported layout transition!" << std::endl;
			return;
		}

		vkCmdPipelineBarrier(
			command_buffer,
			source_stage, destination_stage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
		flush_command_buffer(command_buffer, graphics_queue);
	}

	void device::create_buffer(buffer_object& bo, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)const
	{
		VkBufferCreateInfo buffer_info = {};
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.size = size;
		buffer_info.usage = usage;
		buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VK_CHECK_RESULT(vkCreateBuffer(logical_device, &buffer_info, nullptr, &bo.buffer))

		VkMemoryRequirements mem_requirements;
		vkGetBufferMemoryRequirements(logical_device, bo.buffer, &mem_requirements);

		VkMemoryAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.allocationSize = mem_requirements.size;
		alloc_info.memoryTypeIndex = find_memory_type(gpu, mem_requirements.memoryTypeBits, properties);

		VK_CHECK_RESULT(vkAllocateMemory(logical_device, &alloc_info, nullptr, &bo.buffer_memory))
		VK_CHECK_RESULT(vkBindBufferMemory(logical_device, bo.buffer, bo.buffer_memory, 0))
	}

	void device::create_buffer(buffer_object& bo, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkMemoryAllocateFlags alloc_flags)const
	{
		if (bo.buffer != VK_NULL_HANDLE)
		{
			destroy_buffer(bo);
		}


		VkBufferCreateInfo buffer_info = {};
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.size = size;
		buffer_info.usage = usage;
		buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VK_CHECK_RESULT(vkCreateBuffer(logical_device, &buffer_info, nullptr, &bo.buffer));
		VkMemoryRequirements mem_requirements;
		vkGetBufferMemoryRequirements(logical_device, bo.buffer, &mem_requirements);

		VkMemoryAllocateFlagsInfo memory_alloc_flags{};
		memory_alloc_flags.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		memory_alloc_flags.flags = alloc_flags;

		VkMemoryAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.allocationSize = mem_requirements.size;
		alloc_info.pNext = &memory_alloc_flags;
		alloc_info.memoryTypeIndex = find_memory_type(gpu, mem_requirements.memoryTypeBits, properties);

		VK_CHECK_RESULT(vkAllocateMemory(logical_device, &alloc_info, nullptr, &bo.buffer_memory));
		VK_CHECK_RESULT(vkBindBufferMemory(logical_device, bo.buffer, bo.buffer_memory, 0));
	}

	void device::copy_buffer(VkBuffer dst_buffer, VkBuffer src_buffer, VkDeviceSize size, VkDeviceSize src_offset /*= 0*/, VkDeviceSize dst_offset /*= 0*/) const
	{
		VkCommandBufferAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandPool = pool;
		alloc_info.commandBufferCount = 1;

		VkCommandBuffer command_buffer;

		VK_CHECK_RESULT(vkAllocateCommandBuffers(logical_device, &alloc_info, &command_buffer))

		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VK_CHECK_RESULT(vkBeginCommandBuffer(command_buffer, &begin_info))
		VkBufferCopy copy_region = {};
		copy_region.srcOffset = src_offset;
		copy_region.dstOffset = dst_offset;
		copy_region.size = size;

		vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);
		VK_CHECK_RESULT(vkEndCommandBuffer(command_buffer))
		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffer;

		VK_CHECK_RESULT(vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE))
		VK_CHECK_RESULT(vkQueueWaitIdle(graphics_queue))
		vkFreeCommandBuffers(logical_device, pool, 1, &command_buffer);
	}

	void device::create_image(int width, int height, image_object& io, const image_options& options) const
	{
		VkImageCreateInfo image_info{};
		image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_info.imageType = VK_IMAGE_TYPE_2D;
		image_info.extent.width = width;
		image_info.extent.height = height;
		image_info.extent.depth = 1;
		image_info.mipLevels = 1;
		image_info.arrayLayers = 1;
		image_info.format = options.format;
		image_info.tiling = options.tiling;
		image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		image_info.usage = options.usage;
		image_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VK_CHECK_RESULT(vkCreateImage(logical_device, &image_info, nullptr, &io.image));

		VkMemoryRequirements mem_requirements;
		vkGetImageMemoryRequirements(logical_device, io.image, &mem_requirements);

		VkMemoryAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.allocationSize = mem_requirements.size;
		alloc_info.memoryTypeIndex = find_memory_type(gpu, mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VK_CHECK_RESULT(vkAllocateMemory(logical_device, &alloc_info, nullptr, &io.image_memory));

		vkBindImageMemory(logical_device, io.image, io.image_memory, 0);

		VkImageViewCreateInfo view_info{};
		view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_info.image = io.image;
		view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view_info.format = VK_FORMAT_R8G8B8A8_SRGB;
		view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		view_info.subresourceRange.baseMipLevel = 0;
		view_info.subresourceRange.levelCount = 1;
		view_info.subresourceRange.baseArrayLayer = 0;
		view_info.subresourceRange.layerCount = 1;

		VK_CHECK_RESULT(vkCreateImageView(logical_device, &view_info, nullptr, &io.image_view));

		VkSamplerCreateInfo sampler_info{};
		sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler_info.magFilter = VK_FILTER_LINEAR;
		sampler_info.minFilter = VK_FILTER_LINEAR;
		sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_info.anisotropyEnable = VK_TRUE;
		sampler_info.maxAnisotropy = 16.0f;
		sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		sampler_info.unnormalizedCoordinates = VK_FALSE;
		sampler_info.compareEnable = VK_FALSE;
		sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
		sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler_info.mipLodBias = 0.0f;
		sampler_info.minLod = 0.0f;
		sampler_info.maxLod = 0.0f;
		VK_CHECK_RESULT(vkCreateSampler(logical_device, &sampler_info, nullptr, &io.sampler));
	}

	void device::destroy_buffer(buffer_object& bo) const
	{
		assert(bo.buffer != VK_NULL_HANDLE);
		assert(bo.buffer_memory != VK_NULL_HANDLE);

		vkDestroyBuffer(logical_device, bo.buffer, nullptr);
		vkFreeMemory(logical_device, bo.buffer_memory, nullptr);

		bo.buffer = VK_NULL_HANDLE;
		bo.buffer_memory = VK_NULL_HANDLE;

	}

	void device::destroy_image(image_object& io) const
	{
		vkDestroySampler(logical_device, io.sampler, nullptr);
		vkDestroyImageView(logical_device, io.image_view, nullptr);
		vkDestroyImage(logical_device, io.image, nullptr);
		vkFreeMemory(logical_device, io.image_memory, nullptr);
	}

	bool device::create(const VkInstance& instance, const VkSurfaceKHR& surface)
	{
		gpu = VK_NULL_HANDLE;
		logical_device = VK_NULL_HANDLE;
		if (!pick_physical_device(instance, surface))
		{
			return false;
		}
		pick_logical_device(surface);
		return true;
	}

	void device::create_command_pool()
	{
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = indices.graphics_family.value();
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		VK_CHECK_RESULT(vkCreateCommandPool(logical_device, &poolInfo, nullptr, &pool))
	}

	queue_family_indices device::find_queue_families(const VkPhysicalDevice& device, const VkSurfaceKHR& surface)
	{
		queue_family_indices indices;
		uint32_t queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
		std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);

		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());


		for (uint32_t i = 0; i < queue_family_count; i++)
		{
			if (queue_families[i].queueCount > 0 && queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphics_family = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (queue_families[i].queueCount > 0 && presentSupport)
			{
				indices.present_family = i;
			}
			if (indices.is_complete())
			{
				break;
			}
		}
		return indices;
	}

	bool device::pick_physical_device(const VkInstance& instance, const VkSurfaceKHR& surface)
	{

		uint32_t device_count = 0;
		VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &device_count, nullptr))
		if (device_count == 0)
		{
			return false;
		}

		std::vector<VkPhysicalDevice> devices(device_count);
		VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &device_count, devices.data()))

		for (const auto& device : devices)
		{
			queue_family_indices out_indices;
			swapchain_support_details details;
			if (is_device_suitable(device, surface, &out_indices, &details))
			{
				indices = out_indices;
				gpu = device;
				swapchain_support = details;
				break;
			}
		}

		if (gpu == VK_NULL_HANDLE)
		{
			return false;
		}
		return true;
	}

	void device::pick_logical_device(const VkSurfaceKHR& surface)
	{
		std::vector<VkDeviceQueueCreateInfo> queue_create_infos;

		uint32_t unique_queue_families[]{ indices.graphics_family.value(), indices.present_family.value() };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : unique_queue_families)
		{
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queue_create_infos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceAccelerationStructureFeaturesKHR acceleration_structure_features{};
		acceleration_structure_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
		acceleration_structure_features.accelerationStructure = VK_TRUE;
		acceleration_structure_features.accelerationStructureHostCommands = VK_FALSE;

		VkPhysicalDeviceRayTracingPipelineFeaturesKHR ray_tracing_features{};
		ray_tracing_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
		ray_tracing_features.rayTracingPipeline = VK_TRUE;
		ray_tracing_features.pNext = &acceleration_structure_features;

		VkPhysicalDeviceFeatures device_features{};
		device_features.samplerAnisotropy = VK_TRUE;



		VkPhysicalDeviceVulkan12Features physical_device_vulkan_features{};

		physical_device_vulkan_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		physical_device_vulkan_features.bufferDeviceAddress = VK_TRUE;
		physical_device_vulkan_features.storageBuffer8BitAccess = VK_TRUE;
		physical_device_vulkan_features.descriptorIndexing = VK_TRUE;
		physical_device_vulkan_features.pNext = &ray_tracing_features;


		VkDeviceCreateInfo device_info{};
		device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_info.pNext = &physical_device_vulkan_features;
		device_info.flags = 0;
		device_info.queueCreateInfoCount = 1;
		device_info.pQueueCreateInfos = queue_create_infos.data();
		device_info.enabledExtensionCount = (uint32_t)device_extensions.size();
		device_info.ppEnabledExtensionNames = device_extensions.data();
		device_info.pEnabledFeatures = &device_features;

		VK_CHECK_RESULT(vkCreateDevice(gpu, &device_info, nullptr, &logical_device))
	
		vkGetDeviceQueue(logical_device, indices.graphics_family.value(), 0, &graphics_queue);
		vkGetDeviceQueue(logical_device, indices.present_family.value(), 0, &presentQueue);
	}

	bool device::is_device_suitable(const VkPhysicalDevice& device, const VkSurfaceKHR& surface, queue_family_indices* out_indices, swapchain_support_details* details)
	{
		(*out_indices) = find_queue_families(device, surface);
		bool extensions_supported = check_device_extension_support(device);
		bool swapchain_suitable = false;

		if (extensions_supported)
		{
			(*details) = swapchain_support_details::query_swapchain_support(device, surface);
			swapchain_suitable = !details->formats.empty() && !details->presentModes.empty();
		}

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
		return out_indices->is_complete() && extensions_supported && swapchain_suitable && supportedFeatures.samplerAnisotropy;
	}


	bool device::check_device_extension_support(VkPhysicalDevice device)
	{
		uint32_t extension_count;
		VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr))

		if (extension_count == 0 && device_extensions.size() != 0)
		{
			return false;
		}

		VkExtensionProperties* available = new VkExtensionProperties[extension_count];

		VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available))
		bool missing_extension = false;

		for (uint32_t i = 0; i < device_extensions.size(); i++)
		{
			for (uint32_t j = 0; j < extension_count; j++)
			{
				if (strcmp(device_extensions[i], available[j].extensionName) == 0)
				{
					break;
				}

				if (j == extension_count - 1)
				{
					missing_extension = true;
				}
			}
		}
		delete[] available;
		return !missing_extension;
	}

}