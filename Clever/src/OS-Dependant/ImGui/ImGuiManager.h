#pragma once
#include "ImGuiSrc/imgui.h"
#include "ImGuiSrc/imgui_impl_glfw.h"
#include "ImGuiSrc/imgui_impl_vulkan.h"
#include <memory>
#include <OS-Dependant/Vulkan/VulkanInstance.h>


class ImGuiManager
{
public:

	void bind(std::shared_ptr<VulkanInstance> p_VulkanInstance, uint32_t m_CurrentFrame)
	{

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}
		//DescriptorPool
		{
			VkDescriptorPoolSize pool_sizes[] =
			{
				{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
			};

			VkDescriptorPoolCreateInfo pool_info = {};
			pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
			pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
			pool_info.pPoolSizes = pool_sizes;
			if (vkCreateDescriptorPool(p_VulkanInstance->m_Device, &pool_info, NULL, &m_ImGuiDesciptorPool) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to Create IMGUI Descriptor Pool!");
			}

		}
		
		//RenderPass
		{
			VkAttachmentDescription colorAttachment{};
			colorAttachment.format = VK_FORMAT_B8G8R8A8_UNORM;
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			VkAttachmentReference attachmentReference{};
			attachmentReference.attachment = 0;
			attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpassDesciption{};
			subpassDesciption.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassDesciption.colorAttachmentCount = 1;
			subpassDesciption.pColorAttachments = &attachmentReference;

			VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcAccessMask = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			std::array<VkAttachmentDescription, 1> attachments = { colorAttachment };

			VkRenderPassCreateInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			renderPassInfo.pAttachments = attachments.data();
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpassDesciption;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;
		
		

			if (vkCreateRenderPass(p_VulkanInstance->m_Device, &renderPassInfo, nullptr, &m_ImGuiRenderPass) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create ImGui RenderPass!");
			}
		}
		
		//CommandPool
		{

			VkCommandPoolCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			info.queueFamilyIndex = p_VulkanInstance->queueFamilyIndicies.graphicsIndex.value();

			if (vkCreateCommandPool(p_VulkanInstance->m_Device, &info, nullptr, &m_ImGuiCommandPool) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create Im Gui Command Pool");
			}
		}

		//Command Buffers
		{
			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = m_ImGuiCommandPool;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = p_VulkanInstance->m_max_frames_in_flight;

			m_ImGuiCommandBuffers.resize(p_VulkanInstance->m_max_frames_in_flight);
			if (vkAllocateCommandBuffers(p_VulkanInstance->m_Device, &allocInfo, m_ImGuiCommandBuffers.data()) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to Allocate IMGUI command Buffers");
			}
		}

		//Frame Buffers
		{
			m_ImGuiFrameBuffers.resize(p_VulkanInstance->m_ImageCount);
			for (int i = 0; i < p_VulkanInstance->m_ImageCount; i++)
			{
				VkFramebufferCreateInfo info{};
				info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				info.renderPass = m_ImGuiRenderPass;
				info.attachmentCount = 1;
				info.pAttachments = &p_VulkanInstance->m_ImageViews[i];
				info.width = p_VulkanInstance->m_SwapChainExtent.width;
				info.height = p_VulkanInstance->m_SwapChainExtent.height;
				info.layers = 1;

				if (vkCreateFramebuffer(p_VulkanInstance->m_Device, &info, nullptr, &m_ImGuiFrameBuffers[i]) != VK_SUCCESS)
				{
					throw std::runtime_error("Failed to create IM GUi Fraem Buffer: " + i);
				}
			}
		}

		ImGui_ImplGlfw_InitForVulkan(p_VulkanInstance->getGLFWwindow(), true);
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = p_VulkanInstance->m_Instance;
		init_info.PhysicalDevice = p_VulkanInstance->m_PhysicalDevice;
		init_info.Device = p_VulkanInstance->m_Device;
		init_info.QueueFamily = p_VulkanInstance->queueFamilyIndicies.graphicsIndex.value();
		init_info.Queue = p_VulkanInstance->m_GraphicsQueue;
		init_info.PipelineCache = VK_NULL_HANDLE;
		init_info.DescriptorPool = m_ImGuiDesciptorPool;
		init_info.Allocator = nullptr;
		init_info.MinImageCount = p_VulkanInstance->m_max_frames_in_flight;
		init_info.ImageCount = p_VulkanInstance->m_max_frames_in_flight;
		ImGui_ImplVulkan_Init(&init_info, m_ImGuiRenderPass);

		VkCommandBuffer commandBuffer = p_VulkanInstance->beginSingleTimeCommands();
		ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
		p_VulkanInstance->endSingleTimeCommands(commandBuffer);
	}

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

	void showDemoWindow()
	{
		ImGui::ShowDemoWindow();
	}

	void ImGuiCustomRender()
	{
		
	}

	uint32_t render(std::shared_ptr<VulkanInstance> p_VulkanInstance, uint32_t m_CurrentFrame)
	{
		vkWaitForFences(p_VulkanInstance->m_Device, 1, &p_VulkanInstance->m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;

		VkResult result = vkAcquireNextImageKHR
		(
			p_VulkanInstance->m_Device,
			p_VulkanInstance->m_SwapChain,
			UINT64_MAX,
			p_VulkanInstance->m_ImageAvailableSemaphores[m_CurrentFrame],
			VK_NULL_HANDLE,
			&imageIndex
		);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			//recreateSwapChain();
			return -1;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		vkResetCommandBuffer(m_ImGuiCommandBuffers[m_CurrentFrame], 0);

		ImGui::Render();
		VkCommandBufferBeginInfo info{};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(m_ImGuiCommandBuffers[m_CurrentFrame], &info);

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_ImGuiRenderPass;
		renderPassInfo.framebuffer = m_ImGuiFrameBuffers[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = p_VulkanInstance->m_SwapChainExtent;

		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(m_ImGuiCommandBuffers[m_CurrentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_ImGuiCommandBuffers[m_CurrentFrame]);

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}

		vkCmdEndRenderPass(m_ImGuiCommandBuffers[m_CurrentFrame]);
		vkEndCommandBuffer(m_ImGuiCommandBuffers[m_CurrentFrame]);
		
		return imageIndex;
	}

	void cleanup(std::shared_ptr<VulkanInstance> p_VulkanInstance)
	{
		vkDestroyDescriptorPool(p_VulkanInstance->m_Device, m_ImGuiDesciptorPool, nullptr);
		vkDestroyRenderPass(p_VulkanInstance->m_Device, m_ImGuiRenderPass, nullptr);
		for(int i = 0 ; i < m_ImGuiFrameBuffers.size(); i++)
			vkDestroyFramebuffer(p_VulkanInstance->m_Device, m_ImGuiFrameBuffers[i], nullptr);
		vkDestroyCommandPool(p_VulkanInstance->m_Device, m_ImGuiCommandPool, nullptr);
	}

	private:
		VkDescriptorPool m_ImGuiDesciptorPool;
		VkRenderPass m_ImGuiRenderPass;

		VkCommandPool m_ImGuiCommandPool;
		std::vector<VkCommandBuffer> m_ImGuiCommandBuffers;

		std::vector<VkFramebuffer> m_ImGuiFrameBuffers;

};