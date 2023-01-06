#pragma once
#include "Clever/WorldManager/Vertex.h"
#include <vulkan/vulkan.h>
#include "Clever/WorldManager/UniformBufferObject.h"
#include <fstream>
#include <vector>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

struct PushConstants {
	glm::mat4 model;
};

//template<typename T>
class PipelineInfo
{
public:

	PipelineInfo()
	{

	}
	PipelineInfo(VkDevice device, VkRenderPass renderPass, std::vector<VkBuffer> uniformBuffer, int maxFramesInFlight, bool ray)
		: m_Device(device), m_RenderPass(renderPass), m_UniformBuffers(uniformBuffer), m_MaxFramesInFlight(maxFramesInFlight)
	{
		createPipelineLayout();
		createGraphicsPipeline(ray);
		createDesciptor();
		createPushConstants();
	}
	~PipelineInfo()
	{

	}

	void setInstanceCount(int count)
	{
		positions.resize(count);
	}

	void setPosition(glm::vec3 pos, int instance)
	{
		if (positions.size() < instance)
			return;

		positions.at(instance) = pos;
		createPushConstants();
	}

	int getInstanceCount()
	{
		return static_cast<int>(positions.size());
	}

	void cleanup()
	{
		vkDestroyDescriptorSetLayout(m_Device, descriptorSetLayout, nullptr);
		vkDestroyDescriptorPool(m_Device, descriptorPool, nullptr);
		vkDestroyPipelineLayout(m_Device, pipelineLayout, nullptr);
		vkDestroyPipeline(m_Device, graphicsPipeline, nullptr);

	}

private:
	void createPipelineLayout()
	{
		std::vector< VkDescriptorSetLayoutBinding> bindingInfos(1);
		bindingInfos[0].binding = 0;
		bindingInfos[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bindingInfos[0].descriptorCount = 1;
		bindingInfos[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutCreateInfo desciptorInfo{};
		desciptorInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		desciptorInfo.bindingCount = static_cast<uint32_t>(bindingInfos.size());
		desciptorInfo.pBindings = bindingInfos.data();

		if (vkCreateDescriptorSetLayout(m_Device, &desciptorInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
		{
			std::runtime_error("DesctiptorSetLayout failed to be created!");
		}

		VkPushConstantRange push_constant;
		push_constant.offset = 0;
		push_constant.size = sizeof(PushConstants);
		push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkPipelineLayoutCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		info.setLayoutCount = 1;
		info.pSetLayouts = &descriptorSetLayout;
		info.pushConstantRangeCount = 1;
		info.pPushConstantRanges = &push_constant;

		if (vkCreatePipelineLayout(m_Device, &info, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			std::runtime_error("Pipeline Layout failed to be created!");
		}
	}

	void createGraphicsPipeline(bool ray)
	{
		auto vertShaderCode = readFile("Clever/src/OS-Dependant/Shaders/vert.spv");
		auto fragShaderCode = readFile("Clever/src/OS-Dependant/Shaders/frag.spv");

		VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStageCreateInfo[] = { vertShaderStageInfo, fragShaderStageInfo };

		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());

		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		if (ray)
		{
			rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
			rasterizer.lineWidth = 4.0f;
		}
		else {
			rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizer.lineWidth = 1.0f;
		}
		
		//rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;

		VkGraphicsPipelineCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		createInfo.stageCount = 2;
		createInfo.pStages = shaderStageCreateInfo;
		createInfo.pVertexInputState = &vertexInputInfo;
		createInfo.pInputAssemblyState = &inputAssembly;
		createInfo.pViewportState = &viewportState;
		createInfo.pRasterizationState = &rasterizer;
		createInfo.pMultisampleState = &multisampling;
		createInfo.pColorBlendState = &colorBlending;
		createInfo.pDynamicState = &dynamicState;
		createInfo.pDepthStencilState = &depthStencil;
		createInfo.layout = pipelineLayout;
		createInfo.renderPass = m_RenderPass;
		createInfo.subpass = 0;
		createInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
		{
			std::runtime_error("Failed to create Graphics Pipeline");
		}

		vkDestroyShaderModule(m_Device, fragShaderModule, nullptr);
		vkDestroyShaderModule(m_Device, vertShaderModule, nullptr);
	}

	void createDesciptor()
	{
		//Descriptor Pool
		{
			VkDescriptorPoolSize sizeInfo{};
			sizeInfo.descriptorCount = static_cast<uint32_t>(m_MaxFramesInFlight);
			sizeInfo.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

			VkDescriptorPoolCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			info.poolSizeCount = 1;
			info.pPoolSizes = &sizeInfo;
			info.maxSets = static_cast<uint32_t>(m_MaxFramesInFlight);

			if (vkCreateDescriptorPool(m_Device, &info, nullptr, &descriptorPool) != VK_SUCCESS)
			{
				std::runtime_error("Failed to create descriptor pool");
			}
		}

		//Desciptor Sets
		{
			std::vector<VkDescriptorSetLayout> layouts(m_MaxFramesInFlight, descriptorSetLayout);
			VkDescriptorSetAllocateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			info.descriptorPool = descriptorPool;
			info.descriptorSetCount = static_cast<uint32_t>(m_MaxFramesInFlight);
			info.pSetLayouts = layouts.data();

			descriptorSets.resize(m_MaxFramesInFlight);

			if (vkAllocateDescriptorSets(m_Device, &info, descriptorSets.data()) != VK_SUCCESS)
			{
				std::runtime_error("Falied to allocate Descriptor sets");
			}
		}

		for (size_t i = 0; i < m_MaxFramesInFlight; i++) {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = m_UniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			VkWriteDescriptorSet writeInfo{};
			writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeInfo.dstSet = descriptorSets[i];
			writeInfo.dstBinding = 0;
			writeInfo.dstArrayElement = 0;
			writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeInfo.descriptorCount = 1;
			writeInfo.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(m_Device, 1, &writeInfo, 0, nullptr);
		}
	}

	void createPushConstants()
	{
		VkPushConstantRange push_constant;
		push_constant.offset = 0;
		push_constant.size = sizeof(PushConstants);
		push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		instances.clear();
		instances.resize(positions.size());

		for (int i = 0; i < instances.size(); i++)
		{
			PushConstants push = {};
			push.model = glm::translate(glm::mat4(1.0f), positions.at(i));
			instances[i] = push;
		}
	}

private:
	std::vector<char> readFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		return buffer;
	}
	VkShaderModule createShaderModule(const std::vector<char>& code)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
		VkShaderModule shaderModule;
		if (vkCreateShaderModule(m_Device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Shader Module");
		}

		return shaderModule;
	}

private:
	VkDescriptorSetLayout descriptorSetLayout;//
	VkDescriptorPool descriptorPool;//

	VkDevice m_Device;
	VkRenderPass m_RenderPass;
	std::vector<VkBuffer> m_UniformBuffers;
	int m_MaxFramesInFlight;

	std::vector<glm::vec3> positions;

public:
	VkPipeline graphicsPipeline;//
	VkPipelineLayout pipelineLayout;//
	std::vector<VkDescriptorSet> descriptorSets;//
	std::vector<PushConstants> instances;//! Replace with T when template is added back
};