#include "CommonInitilizers.h"

namespace CVulkan
{
	void createInstance(VkInstance* instanceHandle, const char** glfwExtensions, uint32_t glfwExtensionCount, std::vector<const char*> desiredExtensions)
	{
		VkApplicationInfo appInfo{};//InfoSctruct for applicaiton use
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Clever-Engine";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Clever";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInstanceInfo{};//Struct for creation of Instance, need to fill with required extensions when neccessary
		createInstanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInstanceInfo.pApplicationInfo = &appInfo;

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);//GLFW extensions
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);//For Validation Layers
		extensions.insert(extensions.end(), desiredExtensions.begin(), desiredExtensions.end());//This creates the Total List of desired Extensions both user asked and GLFW asked


		createInstanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInstanceInfo.ppEnabledExtensionNames = extensions.data();

		createInstanceInfo.enabledLayerCount = static_cast<uint32_t>(Constants::validationLayers.size());
		createInstanceInfo.ppEnabledLayerNames = Constants::validationLayers.data();


		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		Helper::populateDebugMessengerCreateInfo(debugCreateInfo);
		createInstanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;

		if (vkCreateInstance(&createInstanceInfo, nullptr, instanceHandle) != VK_SUCCESS)
		{
			//Clever::Log::CRASH("Vulkan Instance did not create successfully");
			throw std::runtime_error("failed to create instance!");
		}
	}
	
	void createPhysicalDevice(VkPhysicalDevice& physicalDevice, Helper::QueueFamilyIndicies& queueFamilyIndicies, VkInstance instance)
	{
		uint32_t physicalDeviceCount = 0;

		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, NULL);//This returns the device count and stores it in "PhysicalDeviceCount"

		std::vector<VkPhysicalDevice> possiblePhysicalDevices;
		std::vector<VkPhysicalDeviceProperties> physicalDeviceProperties;

		possiblePhysicalDevices.resize(physicalDeviceCount);//Stores the PhysicalDevice Handle
		physicalDeviceProperties.resize(physicalDeviceCount);//Stores the PhysicalDevice Properties, in the same order.

		if (vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, possiblePhysicalDevices.data()) != VK_SUCCESS)//Fills vector with possbile Phyiscal Devices
		{
			//Clever::Log::CRASH("Vulkan Instance did not create successfully");
			throw std::runtime_error("failed to enumerate Physical Devices!");
		}

		for (auto pDevice : possiblePhysicalDevices)
		{
			//! This fills queueFamilyProperties with Queue Family Properties. which each contain a queue type(s) and count.
			Helper::QueueFamilyIndicies qfi = Helper::getPhysicalDeviceProperties(pDevice);
			if (qfi.isComplete())
			{
				physicalDevice = pDevice;
				queueFamilyIndicies = qfi;
				break;
			}
		}
		if (physicalDevice == NULL)
		{
			//Clever::Log::CRASH("Vulkan Instance did not create successfully");
			throw std::runtime_error("failed to find usable GPU!");
		}
	}

	void createDevice(VkDevice* deviceHandle, VkPhysicalDevice physicalDevice, std::vector<const char*> desiredExtensions)
	{
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfo;

		uint32_t queueFamilyPropertyCount = 0;
		std::vector<VkQueueFamilyProperties> queueFamilyProperties;

		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, nullptr);

		queueFamilyProperties.resize(queueFamilyPropertyCount);

		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, queueFamilyProperties.data());

		bool graphicsQueue = false;
		bool presentQueue = false;

		Helper::QueueFamilyIndicies QFI = Helper::getPhysicalDeviceProperties(physicalDevice);

		float queuePriority = 1.0f;
		std::set<uint32_t> uniqueIndicies = { QFI.graphicsIndex.value(), QFI.presentIndex.value() };
		for (auto index : uniqueIndicies)
		{
			VkDeviceQueueCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			info.queueFamilyIndex = index;
			info.queueCount = 1;
			info.pQueuePriorities = &queuePriority;
			queueCreateInfo.push_back(info);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		deviceFeatures.fillModeNonSolid = true;
		deviceFeatures.wideLines = true;

		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfo.size());
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfo.data();
		deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(Constants::deviceExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = Constants::deviceExtensions.data();
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(Constants::validationLayers.size());
		deviceCreateInfo.ppEnabledLayerNames = Constants::validationLayers.data();

		if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, deviceHandle) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create Logical Device!");
		}

		/*vkGetDeviceQueue(m_Device, queueFamilyIndicies.graphicsIndex.value(), 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_Device, queueFamilyIndicies.presentIndex.value(), 0, &m_PresentQueue);*/
	}

}