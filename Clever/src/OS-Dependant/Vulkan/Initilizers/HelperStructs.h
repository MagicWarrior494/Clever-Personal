#pragma once
#include <optional>

namespace Helper
{
	struct QueueFamilyIndicies
	{
		std::optional<uint32_t> graphicsIndex;
		std::optional<uint32_t> presentIndex;

		bool isComplete()
		{
			return graphicsIndex.has_value() && presentIndex.has_value();
		}
	};
}