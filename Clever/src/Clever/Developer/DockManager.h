#pragma once
#include "DevTools.h"

namespace DevTools
{
	//This Controls All the UI DOCKS
	static void BuildCustomUI()
	{
		for (const auto& [key, value] : DevTools::docks)
		{
			key(value);
		}
	}
}