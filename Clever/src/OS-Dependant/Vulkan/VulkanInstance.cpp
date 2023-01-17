#include "VulkanInstance.h"


VulkanInstance::VulkanInstance(int height, int width, int max_frames_in_flight, bool RTXEnable) :
	m_SwapChainExtent({ static_cast<uint32_t>(height), static_cast<uint32_t>(width) }), m_max_frames_in_flight(max_frames_in_flight)
{
	createInstance();
	m_Camera.reset(new Camera(90.0f, height, width, 0.1f, 1000.0f, glm::vec3(3, 1, 8), m_Window));
	m_Camera->init();
}

void VulkanInstance::cleanup()
{
		cleanupSwapChain();

		for (size_t i = 0; i < m_max_frames_in_flight; i++)
		{
			vkDestroyBuffer(m_Device, m_UniformBuffers[i], nullptr);
			vkFreeMemory(m_Device, m_UniformBuffersMemory[i], nullptr);
		}

		for (size_t i = 0; i < m_max_frames_in_flight; i++) {
			vkDestroySemaphore(m_Device, m_RenderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(m_Device, m_ImageAvailableSemaphores[i], nullptr);
			vkDestroyFence(m_Device, m_InFlightFences[i], nullptr);
		}

		vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);

		vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);

		vkDestroyDevice(m_Device, nullptr);

		vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);



		//! Destroys Validation Layers
		{
			auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT");
			if (func != nullptr) {
				func(m_Instance, m_DebugMessenger, nullptr);
			}
		}

		vkDestroyInstance(m_Instance, nullptr);

		glfwDestroyWindow(m_Window);
		glfwTerminate();
}

bool VulkanInstance::render(float time, Renderable* renderStart, VkCommandBuffer ImGuiCommandBuffer, uint32_t count, uint32_t m_CurrentFrame, uint32_t imageIndex)
{
	vkWaitForFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

	if (imageIndex == uint32_t(-1))
	{
		VkResult result = vkAcquireNextImageKHR
		(
			m_Device,
			m_SwapChain,
			UINT64_MAX,
			m_ImageAvailableSemaphores[m_CurrentFrame],
			VK_NULL_HANDLE,
			&imageIndex
		);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return true;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}
	}

	m_Camera->update(time);
	updateUniformBuffer(m_CurrentFrame, time);

	vkResetFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame]);

	vkResetCommandBuffer(m_CommandBuffers[m_CurrentFrame], 0);

	recordCommandBuffer(imageIndex, renderStart, count, m_CurrentFrame);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	std::vector< VkCommandBuffer> submitCommandBuffers;

	if (ImGuiCommandBuffer == VK_NULL_HANDLE)
	{
		submitCommandBuffers.resize(1);
		submitCommandBuffers[0] = m_CommandBuffers[m_CurrentFrame];
	}
	else
	{
		submitCommandBuffers.resize(2);
		submitCommandBuffers[0] = m_CommandBuffers[m_CurrentFrame];
		submitCommandBuffers[1] = ImGuiCommandBuffer;
	}


	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = static_cast<uint32_t>(submitCommandBuffers.size());
	submitInfo.pCommandBuffers = submitCommandBuffers.data();

	VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { m_SwapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &imageIndex;

	VkResult result = vkQueuePresentKHR(m_PresentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_FramebufferResized) {
		m_FramebufferResized = false;
		//recreateSwapChain();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}
	return false;
}

void VulkanInstance::recordCommandBuffer(uint32_t imageIndex, Renderable* renderStart, uint32_t count, uint32_t m_CurrentFrame)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(m_CommandBuffers[m_CurrentFrame], &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = m_ClearValue.color;
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_RenderPass;
	renderPassInfo.framebuffer = m_FrameBuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0,0 };
	renderPassInfo.renderArea.extent = m_SwapChainExtent;
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_SwapChainExtent.width);
	viewport.height = static_cast<float>(m_SwapChainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(m_CommandBuffers[m_CurrentFrame], 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_SwapChainExtent;
	vkCmdSetScissor(m_CommandBuffers[m_CurrentFrame], 0, 1, &scissor);

	vkCmdBeginRenderPass(m_CommandBuffers[m_CurrentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	for (uint32_t i = 0; i < count; i++)
	{
		Renderable* renderData = (renderStart + (int)i);
		VkBuffer vertexBuffers[] = { renderData->meshData.vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindPipeline(m_CommandBuffers[m_CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, renderData->pipelineInfo.graphicsPipeline);

		vkCmdBindVertexBuffers(m_CommandBuffers[m_CurrentFrame], 0, 1, vertexBuffers, offsets);

		vkCmdBindIndexBuffer(m_CommandBuffers[m_CurrentFrame], renderData->meshData.indexBuffer, 0, VK_INDEX_TYPE_UINT16);

		vkCmdBindDescriptorSets(m_CommandBuffers[m_CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, renderData->pipelineInfo.pipelineLayout, 0, 1, &renderData->pipelineInfo.descriptorSets[m_CurrentFrame], 0, nullptr);

		for (int x = 0; x < renderData->pipelineInfo.instances.size(); x++)
		{
			vkCmdPushConstants(m_CommandBuffers[m_CurrentFrame], renderData->pipelineInfo.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(renderData->pipelineInfo.instances[x]), &renderData->pipelineInfo.instances[x]);

			vkCmdDrawIndexed(m_CommandBuffers[m_CurrentFrame], static_cast<uint32_t>(renderData->meshData.getIndexCount()), 1, 0, 0, 0);
		}
	}

	vkCmdEndRenderPass(m_CommandBuffers[m_CurrentFrame]);

	if (vkEndCommandBuffer(m_CommandBuffers[m_CurrentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}

void VulkanInstance::recreateSwapChain()
{
	int width = 0, height = 0;
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(m_Window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(m_Device);

	cleanupSwapChain();

	createSwapChain();
	createImageViews();
	createDepthResources();
	createFramebuffers();
}

void VulkanInstance::cleanupSwapChain() {
	vkDestroyImageView(m_Device, depthImageView, nullptr);
	vkDestroyImage(m_Device, depthImage, nullptr);
	vkFreeMemory(m_Device, depthImageMemory, nullptr);

	for (auto framebuffer : m_FrameBuffers) {
		vkDestroyFramebuffer(m_Device, framebuffer, nullptr);
	}

	for (auto imageView : m_ImageViews) {
		vkDestroyImageView(m_Device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(m_Device, m_SwapChain, nullptr);
}

void VulkanInstance::createSwapChain()
{

	SwapChainSupportDetails swapChainSupport;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalDevice, m_Surface, &swapChainSupport.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, m_Surface, &formatCount, nullptr);

	if (formatCount != 0) {
		swapChainSupport.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, m_Surface, &formatCount, swapChainSupport.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, m_Surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		swapChainSupport.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, m_Surface, &presentModeCount, swapChainSupport.presentModes.data());
	}

	m_SwapChainExtent.width = std::clamp(m_SwapChainExtent.width, swapChainSupport.capabilities.minImageExtent.width, swapChainSupport.capabilities.maxImageExtent.width);
	m_SwapChainExtent.height = std::clamp(m_SwapChainExtent.height, swapChainSupport.capabilities.minImageExtent.height, swapChainSupport.capabilities.maxImageExtent.height);


	VkSwapchainCreateInfoKHR swapchainCreateInfo{};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = m_Surface;
	swapchainCreateInfo.minImageCount = swapChainSupport.capabilities.minImageCount + 1;
	swapchainCreateInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
	swapchainCreateInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	swapchainCreateInfo.imageExtent = m_SwapChainExtent;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	uint32_t queueFamilyIndices[] = { queueFamilyIndicies.graphicsIndex.value(), queueFamilyIndicies.presentIndex.value() };

	if (queueFamilyIndicies.graphicsIndex != queueFamilyIndicies.presentIndex)
	{
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainCreateInfo.queueFamilyIndexCount = 2;
		swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCreateInfo.queueFamilyIndexCount = 0;
		swapchainCreateInfo.pQueueFamilyIndices = nullptr;
	}

	swapchainCreateInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(m_Device, &swapchainCreateInfo, nullptr, &m_SwapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create Swap Chain!");
	}

}

void VulkanInstance::createImageViews()
{
	vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &m_ImageCount, nullptr);
	m_SwapChainImages.resize(m_ImageCount);
	m_ImageViews.resize(m_ImageCount);
	vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &m_ImageCount, m_SwapChainImages.data());


	int i = 0;
	for (VkImage image : m_SwapChainImages)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_SwapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(m_Device, &createInfo, nullptr, &m_ImageViews[i]) != VK_SUCCESS)
		{
			std::runtime_error("Unable to make Image View!");
		}
		i++;
	}
}

void VulkanInstance::createDepthResources()
{
	VkFormat depthFormat = findDepthFormat();

	createImage(m_SwapChainExtent.width, m_SwapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
	depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void VulkanInstance::createFramebuffers()
{
	int i = 0;
	m_FrameBuffers.resize(m_ImageViews.size());
	for (VkImageView imageView : m_ImageViews)
	{
		std::array<VkImageView, 2> attachments = {
			imageView,
			depthImageView
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_RenderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = m_SwapChainExtent.width;
		framebufferInfo.height = m_SwapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(m_Device, &framebufferInfo, nullptr, &m_FrameBuffers[i]) != VK_SUCCESS)
		{
			std::runtime_error("Could not create Framebuffer!");
		}
		i++;
	}
}

void VulkanInstance::createInstance()
{
	{
		glfwInit();

		//! Creating Instance, Modify when extensions are needed
		{
			uint32_t glfwExtensionCount;
			const char** glfwExtensions;
			glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
			CVulkan::createInstance(&m_Instance, glfwExtensions, glfwExtensionCount, {});
		}

		//! Setting up validation Layers
		{
			VkDebugUtilsMessengerCreateInfoEXT createInfo;
			Helper::populateDebugMessengerCreateInfo(createInfo);

			VkResult result;

			auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT");
			if (func != nullptr) {
				result = func(m_Instance, &createInfo, nullptr, &m_DebugMessenger);
			}
			else {
				result = VK_ERROR_EXTENSION_NOT_PRESENT;
			}

			if (result != VK_SUCCESS) {
				throw std::runtime_error("failed to set up debug messenger!");
			}
		}

		//! Picking Physical Device
		{
			CVulkan::createPhysicalDevice(m_PhysicalDevice, queueFamilyIndicies, m_Instance);
		}
		
		//! Creating Surface
		{

			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

			//glfwGetPrimaryMonitor() For Fullscreen, second to last nullptr needs to be replaced
			m_Window = glfwCreateWindow(m_SwapChainExtent.width, m_SwapChainExtent.height, "Vulkan", nullptr, nullptr);
			glfwSetWindowUserPointer(m_Window, this);
			//glfwSetFramebufferSizeCallback(m_Window, framebufferResizeCallback);

			if (!m_Window)
			{
				throw std::runtime_error("failed to create GLFW window!");
			}

			if (glfwCreateWindowSurface(m_Instance, m_Window, nullptr, &m_Surface))
			{
				throw std::runtime_error("failed to create window surface!");
			}

			std::cout << "Surface Successfuly Created" << std::endl;
		}

		//! Creating Logical Device
		{
			CVulkan::createDevice(&m_Device, m_PhysicalDevice, {});

			vkGetDeviceQueue(m_Device, queueFamilyIndicies.graphicsIndex.value(), 0, &m_GraphicsQueue);
			vkGetDeviceQueue(m_Device, queueFamilyIndicies.presentIndex.value(), 0, &m_PresentQueue);
		}

		//! Creating SwapChain
		{
			createSwapChain();
		}

		//! Creating ImageViews
		{
			
			createImageViews();
		}

		//! Creating Renderpass
		{
			VkAttachmentDescription colorAttachment{};
			colorAttachment.format = VK_FORMAT_B8G8R8A8_UNORM;
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			VkAttachmentReference attachmentReference{};
			attachmentReference.attachment = 0;
			attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentDescription depthAttachment{};
			depthAttachment.format = findDepthFormat();
			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkAttachmentReference depthAttachmentRef{};
			depthAttachmentRef.attachment = 1;
			depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpassDesciption{};
			subpassDesciption.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassDesciption.colorAttachmentCount = 1;
			subpassDesciption.pColorAttachments = &attachmentReference;
			subpassDesciption.pDepthStencilAttachment = &depthAttachmentRef;

			VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcAccessMask = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

			VkRenderPassCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			createInfo.pAttachments = attachments.data();
			createInfo.subpassCount = 1;
			createInfo.pSubpasses = &subpassDesciption;
			createInfo.dependencyCount = 1;
			createInfo.pDependencies = &dependency;

			if (vkCreateRenderPass(m_Device, &createInfo, nullptr, &m_RenderPass) != VK_SUCCESS)
			{
				std::runtime_error("Failed to create RenderPass");
			}

		}

		//! Creating the Command Pool
		{
			VkCommandPoolCreateInfo commandPoolInfo{};
			commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			commandPoolInfo.queueFamilyIndex = queueFamilyIndicies.graphicsIndex.value();

			if (vkCreateCommandPool(m_Device, &commandPoolInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
				throw std::runtime_error("failed to create Command Pool!");
		}

		//! Uniform Buffers
		{
			//Uniform Buffer
			{
				VkDeviceSize bufferSize = sizeof(UniformBufferObject);

				m_UniformBuffers.resize(m_max_frames_in_flight);
				m_UniformBuffersMemory.resize(m_max_frames_in_flight);
				m_UniformBuffersMapped.resize(m_max_frames_in_flight);

				for (size_t i = 0; i < m_max_frames_in_flight; i++) {
					createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_UniformBuffers[i], m_UniformBuffersMemory[i]);

					vkMapMemory(m_Device, m_UniformBuffersMemory[i], 0, bufferSize, 0, &m_UniformBuffersMapped[i]);
				}
			}
		}

		//! Creating Command Buffers
		{
			m_CommandBuffers.resize(m_max_frames_in_flight);

			VkCommandBufferAllocateInfo allocationInfo{};
			allocationInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocationInfo.commandPool = m_CommandPool;
			allocationInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocationInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

			if (vkAllocateCommandBuffers(m_Device, &allocationInfo, m_CommandBuffers.data()) != VK_SUCCESS)
			{
				std::runtime_error("Unable to allcocate Command Buffers");
			}
		}

		//! Creating Depth Resources
		{
			createDepthResources();
		}

		//! Creating FrameBuffers
		{
			createFramebuffers();
		}

		//! Creating Sync Object
		{
			m_ImageAvailableSemaphores.resize(m_max_frames_in_flight);
			m_RenderFinishedSemaphores.resize(m_max_frames_in_flight);
			m_InFlightFences.resize(m_max_frames_in_flight);

			VkSemaphoreCreateInfo semaphoreInfo{};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			for (size_t i = 0; i < m_max_frames_in_flight; i++)
			{
				if (vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS ||
					vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS ||
					vkCreateFence(m_Device, &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS) {
					throw std::runtime_error("failed to create semaphores!");
				}
			}
		}
	}
}

void VulkanInstance::updateUniformBuffer(uint32_t currentFrame, float time) {

	UniformBufferObject ubo{};
	ubo.viewproj = m_Camera->GetViewProjectionMatrix();
	/*ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f), m_SwapChainExtent.width / (float)m_SwapChainExtent.height, 0.1f, 10.0f);
	ubo.proj[1][1] *= -1;*/
	memcpy(m_UniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));

}