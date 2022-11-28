#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "HelperStructs.h"

namespace Constants
{
	const std::vector<const char*> validationLayers =
	{
		"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char*> deviceExtensions =
	{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
}