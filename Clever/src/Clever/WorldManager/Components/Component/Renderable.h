#pragma once
#include "Component.h"
#include "OS-Dependant/Vulkan/PipelineInfo.h"
#include "Clever/WorldManager/MeshData.h"

struct Renderable : Component
{
	MeshData meshData;// This contains the Vertex and Index Information
	PipelineInfo pipelineInfo;// Graphics pipeline, pipelinelayout, descriptor set, push constant OBject and its list pf data called Instances, 

	Renderable()
	{

	}

	Renderable(VkDevice device, VkPhysicalDevice physicalDevice, VkRenderPass renderPass, VkCommandPool commandPool, VkQueue graphicsQueue, std::vector<VkBuffer>& uniformBuffer, int maxFramesInFlight, bool ray = false)
	{
		meshData = MeshData(device, physicalDevice, commandPool, graphicsQueue);
		pipelineInfo = PipelineInfo(device, renderPass, uniformBuffer, maxFramesInFlight, ray);
		pipelineInfo.setInstanceCount(1);
	}

	void setComponentData(std::vector<Vertex> vertices, std::vector<uint16_t> indices)
	{
		meshData.createVertexBuffer(vertices);
		meshData.createIndexBuffer(indices);
	}
	void setComponentData(std::pair<std::vector<Vertex>, std::vector<uint16_t>> data)
	{
		meshData.createVertexBuffer(data.first);
		meshData.createIndexBuffer(data.second);
	}
	
	void setInstanceCount(int count)
	{
		pipelineInfo.setInstanceCount(count);
	}

	int getInstanceCount()
	{
		return pipelineInfo.getInstanceCount();
	}

	void setLocation(glm::vec3 pos, int instance = 0)
	{
		pipelineInfo.setPosition(pos, instance);
	}

	void destory()
	{
		meshData.cleanup();
		pipelineInfo.cleanup();
	}

	virtual std::string toString() override
	{
		return " HAVE NOT WRITTEN THE OVERRIDE TOSTRING YET!! ";
	}
};