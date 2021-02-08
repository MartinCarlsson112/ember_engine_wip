#include "graphics_pipeline.h"
#include "Vertex.h"
#include "vulkan_utils.h"

namespace em
{
	void graphics_pipeline::create(const VkDevice& logical_device, 
		const std::vector<shader>& shaders, 
		const VkDescriptorSetLayout& descriptor_set_layout, 
		const VkRenderPass& render_pass, 
		const VkViewport& viewport, 
		const VkExtent2D extents,
		const graphics_pipeline_settings& settings)
	{
		std::vector<VkPipelineShaderStageCreateInfo> stage_infos(shaders.size());
		for (int i = 0; i < shaders.size(); i++)
		{
			stage_infos[i].stage = shaders[i].type;
			stage_infos[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			stage_infos[i].pName = "main";
			stage_infos[i].module = shaders[i].shader_module;
		}

		auto binding_description = settings.binding_desc;
		auto attribute_descriptions = settings.attribute_desc;
		VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
		vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_info.vertexBindingDescriptionCount = 1U;
		vertex_input_info.vertexAttributeDescriptionCount = (uint32_t)attribute_descriptions.size();
		vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();
		vertex_input_info.pVertexBindingDescriptions = &binding_description;

		VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
		input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly.topology = settings.primitives;
		input_assembly.primitiveRestartEnable = VK_FALSE;

		VkRect2D scissor = { };
		scissor.extent = extents;
		scissor.offset = { 0,0 };

		VkPipelineViewportStateCreateInfo viewport_state = {};
		viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state.viewportCount = 1;
		viewport_state.pViewports = &viewport;
		viewport_state.scissorCount = 1;
		viewport_state.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = settings.cullmode;
		rasterizer.frontFace = settings.winding;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f;
		rasterizer.depthBiasClamp = 0.0f; 
		rasterizer.depthBiasSlopeFactor = 0.0f; 

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = settings.sample_count;
		multisampling.minSampleShading = 1.0f; 
		multisampling.pSampleMask = nullptr; 
		multisampling.alphaToCoverageEnable = VK_FALSE; 
		multisampling.alphaToOneEnable = VK_FALSE; 

		VkPipelineColorBlendAttachmentState color_blend_attachment = {};
		color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		color_blend_attachment.blendEnable = VK_FALSE;
		color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; 
		color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; 
		color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD; 
		color_blend_attachment.blendEnable = VK_TRUE;
		color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo color_blending = {};
		color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blending.logicOpEnable = VK_FALSE;
		color_blending.logicOp = VK_LOGIC_OP_COPY;
		color_blending.attachmentCount = 1;
		color_blending.pAttachments = &color_blend_attachment;
		color_blending.blendConstants[0] = 0.0f;
		color_blending.blendConstants[1] = 0.0f;
		color_blending.blendConstants[2] = 0.0f;
		color_blending.blendConstants[3] = 0.0f;

		VkPushConstantRange push_constant_range = VkPushConstantRange();
		push_constant_range.size = sizeof(float4x4);
		push_constant_range.offset = 0;
		push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkPushConstantRange material_constant_range = VkPushConstantRange();
		material_constant_range.size = sizeof(uint32_t);
		material_constant_range.offset = sizeof(float4x4);
		material_constant_range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkPushConstantRange constant_ranges[] = {
			push_constant_range, material_constant_range
		};


		VkPipelineDepthStencilStateCreateInfo depth_stencil{};
		depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil.depthTestEnable = VK_TRUE;
		depth_stencil.depthWriteEnable = VK_TRUE;
		depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depth_stencil.depthBoundsTestEnable = VK_FALSE;
		depth_stencil.minDepthBounds = 0.0f; 
		depth_stencil.maxDepthBounds = 1.0f; 
		depth_stencil.stencilTestEnable = VK_FALSE;

		VkPipelineLayoutCreateInfo pipeline_layout = {};
		pipeline_layout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout.setLayoutCount = 1;
		pipeline_layout.pSetLayouts = &descriptor_set_layout;
		pipeline_layout.pushConstantRangeCount = COUNT_OF(constant_ranges);
		pipeline_layout.pPushConstantRanges = constant_ranges;
		VK_CHECK_RESULT(vkCreatePipelineLayout(logical_device, &pipeline_layout, nullptr, &layout));

		VkGraphicsPipelineCreateInfo pipeline_info = {};
		pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_info.stageCount = 2;
		pipeline_info.pStages = stage_infos.data();
		pipeline_info.pVertexInputState = &vertex_input_info;
		pipeline_info.pInputAssemblyState = &input_assembly;
		pipeline_info.pViewportState = &viewport_state;
		pipeline_info.pRasterizationState = &rasterizer;
		pipeline_info.pMultisampleState = &multisampling;
		pipeline_info.pDepthStencilState = &depth_stencil;
		pipeline_info.pColorBlendState = &color_blending;
		pipeline_info.pDynamicState = nullptr; 
		pipeline_info.layout = layout;
		pipeline_info.renderPass = render_pass;
		pipeline_info.subpass = 0;
		pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
		pipeline_info.basePipelineIndex = -1;
		VK_CHECK_RESULT(vkCreateGraphicsPipelines(logical_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline));
	}
}
