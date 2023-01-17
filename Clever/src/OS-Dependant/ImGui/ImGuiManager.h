#pragma once
#include "ImGuiSrc/imgui.h"
#include "ImGuiSrc/imgui_impl_glfw.h"
#include "ImGuiSrc/imgui_impl_vulkan.h"
#include <memory>
#include <OS-Dependant/Vulkan/VulkanInstance.h>


class ImGuiManager
{
public:

	void bind(std::shared_ptr<VulkanInstance> p_VulkanInstance, uint32_t m_CurrentFrame);

	VkCommandBuffer getCommandBuffer(uint32_t m_CurrentFrame)
	{
		return m_ImGuiCommandBuffers[m_CurrentFrame];
	}

	void newFrame()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void endFrame()
	{
		ImGui::EndFrame();
		ImGui::UpdatePlatformWindows();
	}

	void showDemoWindow()
	{
		ImGui::ShowDemoWindow();
	}

	uint32_t render(std::shared_ptr<VulkanInstance> p_VulkanInstance, uint32_t m_CurrentFrame);

	void cleanup(std::shared_ptr<VulkanInstance> p_VulkanInstance);

	void recreateFrameBuffer(std::shared_ptr<VulkanInstance> p_VulkanInstance);

private:

	void createFrameBuffer(std::shared_ptr<VulkanInstance> p_VulkanInstance);

private:
	VkDescriptorPool m_ImGuiDesciptorPool;
	VkRenderPass m_ImGuiRenderPass;

	VkCommandPool m_ImGuiCommandPool;
	std::vector<VkCommandBuffer> m_ImGuiCommandBuffers;

	std::vector<VkFramebuffer> m_ImGuiFrameBuffers;

};