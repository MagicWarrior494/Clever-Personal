#include "VulkanInstance.h"

void VulkanInstance::createInstance(bool RTXEnable)
{
	{
		glfwInit();

		//! Creating Instance, Modify when extensions are needed
		uint32_t glfwExtensionCount;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		CVulkan::createInstance(&m_Instance, glfwExtensions, glfwExtensionCount, {});

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
		CVulkan::createPhysicalDevice(m_PhysicalDevice, queueFamilyIndicies, m_Instance);

		//! Creating Surface
		{

			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

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
			swapchainCreateInfo.imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
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

		//! Creating ImageViews
		{
			uint32_t imageCount;
			vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, nullptr);
			m_SwapChainImages.resize(imageCount);
			m_ImageViews.resize(imageCount);
			vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, m_SwapChainImages.data());


			int i = 0;
			for (VkImage image : m_SwapChainImages)
			{
				VkImageViewCreateInfo createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				createInfo.image = m_SwapChainImages[i];
				createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				createInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
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

		//! Creating Renderpass
		{
			VkAttachmentDescription colorAttachment{};
			colorAttachment.format = VK_FORMAT_B8G8R8A8_SRGB;
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

		//! Creating Commaned Buffers
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
			VkFormat depthFormat = findDepthFormat();

			createImage(m_SwapChainExtent.width, m_SwapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
			depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
		}

		//! Creating FrameBuffers
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
