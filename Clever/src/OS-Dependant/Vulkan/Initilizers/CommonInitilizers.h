#pragma once
#include <vector>

#include "HelperFunctions.h"
#include "HelperStructs.h"
#include "Constants.h"
#include <set>

namespace CVulkan
{

	void createInstance(VkInstance* instanceHandle, const char** glfwExtensions, uint32_t glfwExtensionCount, std::vector<const char*> desiredExtensions = {});

	void createPhysicalDevice(VkPhysicalDevice& physicalDevice, Helper::QueueFamilyIndicies& queueFamilyIndicies, VkInstance instance);

	void createDevice(VkDevice* deviceHandle, VkPhysicalDevice physicalDevice, std::vector<const char*> desiredExtensions = {});
}