#include "EventManager.h"


namespace Event
{

	std::vector<bool> EventManager::keys = std::vector<bool>(InputCodes::KEY_UNDEFINED);

	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		EventManager* eventManager = static_cast<EventManager*>(glfwGetWindowUserPointer(window));

		InputCodes::Keyboard cleverKey = (InputCodes::Keyboard)keyboardGLFWtoCleverKeyCodes.at(key);

		if (action == GLFW_PRESS)
			eventManager->setKey(cleverKey, true);
		else
			eventManager->setKey(cleverKey, false);
	}

	static void mouseButton_callback(GLFWwindow* window, int key, int action, int mods)
	{
		EventManager* eventManager = static_cast<EventManager*>(glfwGetWindowUserPointer(window));

		InputCodes::Mouse cleverMouseButton = (InputCodes::Mouse)mouseGLFWtoCleverButtonCodes.at(key);

		if (action == GLFW_PRESS)
			eventManager->setMouseButton(cleverMouseButton, true);
		else
			eventManager->setMouseButton(cleverMouseButton, false);
	}

	EventManager::EventManager()
	{

	}

	EventManager::~EventManager()
	{
		
	}

	void EventManager::Init(GLFWwindow* window)
	{
		p_Window = window;
		glfwSetWindowUserPointer(window, this);
		glfwSetKeyCallback(window, key_callback);
		glfwSetMouseButtonCallback(window, mouseButton_callback);
	}

	void EventManager::updateWindowPointer(GLFWwindow* window)
	{
		p_Window = window;
	}

	void EventManager::UpdateKeyPresses()
	{
		glfwPollEvents();

		for (const auto& [key, value] : keyboardGLFWtoCleverKeyCodes)
		{
			if (glfwGetKey(p_Window, key))
				keys[value] = true;
			else
				keys[value] = false;
		}
	}

	float EventManager::getTime()
	{
		return (float)glfwGetTime();
	}

	std::pair<int, int> EventManager::getMousePos()
	{
		std::pair<double, double> pos;
		glfwGetCursorPos(p_Window, &pos.first, &pos.second);
		return pos;
	}

	void EventManager::SubscribeFunction(KeySet* requiredKeysAndButtons, void* function, void* objectPointer, int priority)
	{
		if (subscriptionKeysAndButtonstoFunctionMapping.find(requiredKeysAndButtons) != subscriptionKeysAndButtonstoFunctionMapping.end())
		{
			//Keys have already been added

			subscriptionKeysAndButtonstoFunctionMapping.at(requiredKeysAndButtons).insert({ priority, std::pair<void (*)(void*), void*>((void (*)(void*))function, objectPointer) });

		}
		else
		{
			//Keys have not been added to the mapping

			subscriptionKeysAndButtonstoFunctionMapping.insert({ requiredKeysAndButtons,  {{ priority, std::pair<void (*)(void*), void*>((void (*)(void*))function, objectPointer) }}});

		}

	}

	//bool EventManager::isKeyPressed(InputCodes::Keyboard code)
	

	bool EventManager::isMouseButtonPressed(InputCodes::Mouse code)
	{
		return mouseButtons[code];
	}

	void EventManager::HandleSubscriberCalls()
	{
		//Checking all key Subscibers
		for (const auto& [KeycodeSet, mappedFunctions] : subscriptionKeysAndButtonstoFunctionMapping)
		{
			bool areKeysPressed = true;
			for (auto& key : KeycodeSet->keys)
			{
				if (!isKeyPressed(key))
				{
					areKeysPressed = false;
					break;
				}
			}

			if (!areKeysPressed)
				continue;

			for (auto& key : KeycodeSet->mouseButtons)
			{
				if (!isMouseButtonPressed(key))
				{
					areKeysPressed = false;
					break;
				}
			}

			if (!areKeysPressed)
				continue;

			for (const auto& [priority, functionsPair] : mappedFunctions)
			{
				const auto& function = functionsPair.first;
				const auto& objectPointer = functionsPair.second;

				function(objectPointer);
			}
		}
	}
}