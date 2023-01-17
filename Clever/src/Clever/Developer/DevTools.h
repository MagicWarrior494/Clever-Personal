#pragma once
#include <string>
#include <vector>
#include "OS-Dependant/ImGui/ImGuiSrc/imgui.h"
#include <unordered_map>
#include <glm.hpp>

namespace DevTools
{
	static std::unordered_map<void (*)(std::vector<void*>), std::vector<void*>> docks;

	static void newDock(std::string name)
	{
		ImGui::Begin(name.c_str());
	}

	static void coloredText(glm::vec3 color, std::string text)
	{
		ImGui::TextColored(ImVec4(color.r, color.g, color.b, 1.0f), text.c_str());
	}

	static bool button(std::string label, glm::vec2 size = {0,0})
	{
		return ImGui::Button(label.c_str(), ImVec2(size.x, size.y));
	}

	static void floatSlider(std::string label, float* value, float min, float max)
	{
		ImGui::SliderFloat(label.c_str(), value, min, max, "%f", 1);
	}

	static void endDock()
	{
		ImGui::End();
	}

	static void addDockFunction(void* func, std::vector<void*> classPointer)
	{
		docks.insert({ ((void (*)(std::vector<void*>))func), classPointer });
	}
}