project "vulkan"
	kind "StaticLib"
	language "C"
	staticruntime "off"

	targetdir ("bingfd/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"Include/**/.h",
		"Include/**/.hpp"
	}

	links
	{
		"Lib/vulkan-1.lib"
	}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
