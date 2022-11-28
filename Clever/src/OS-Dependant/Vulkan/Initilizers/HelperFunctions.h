#pragma once
#include <iostream>
#include <vulkan/vulkan.h>
#include <vector>

#include "HelperStructs.h"

namespace Helper
{

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return false;
	}

	Helper::QueueFamilyIndicies getPhysicalDeviceProperties(VkPhysicalDevice physicalDevice);

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
}