#pragma once
#include "CleverKeyCodes.h"
#include "OS-Dependant/GLFW/GLFWConversionTable.h"
#include <unordered_map>
#include <unordered_map>
#include <set>
#include <vector>
//#include "Clever/Entry/Clever.h"

namespace Event
{
	struct KeySet
	{
		std::vector<InputCodes::Keyboard> keys;
		std::vector<InputCodes::Mouse> mouseButtons;
	};


	class EventManager
	{
	public:
		EventManager();
		~EventManager();

		void Init(GLFWwindow* window);
		void updateWindowPointer(GLFWwindow* window);

		void UpdateKeyPresses();

		float getTime();

		static bool isKeyPressed(InputCodes::Keyboard code)
		{
			return keys.at(code);
		}

		bool isMouseButtonPressed(InputCodes::Mouse code);
		void SubscribeFunction(KeySet* requiredKeysAndButtons, void* function, void* objectPointer, int priority = 0); //Bigger priority will be called first

		std::pair<int, int> getMousePos();

		void HandleSubscriberCalls();

		void setKey(InputCodes::Keyboard key, bool state)
		{
			keys.at(key) = state;
		}

		void setMouseButton(InputCodes::Mouse button, bool state)
		{
			mouseButtons[button] = state;
		}

	private:
		GLFWwindow* p_Window;

		static std::vector<bool> keys;
		bool mouseButtons[InputCodes::BUTTON_UNDEFINED];

		std::unordered_map<KeySet*, std::unordered_map<int, std::pair<void (*)(void*), void*>>> subscriptionKeysAndButtonstoFunctionMapping;
		//Set of Key Codes to map of priority to function pointer with object referneces so they arent static

	};
}

