#pragma once
#include "OS-Dependant/Vulkan/VulkanInstance.h"
#include "Clever/Camera/Camera.h"
#include "Clever/WorldManager/Components/Component/Renderable.h"
#include <memory>

namespace Window
{
	class WindowManager
	{
	public:
		struct WindowFlags
		{
			int width;
			int height;
			int max_frames_in_flight;
			bool RTXEnable;
		};

	private:
		
	public:
		WindowManager()
		{

		}
		void WindowInit(WindowFlags flags = {})
		{
			m_Flags = flags;
			m_VulkanInstance.reset(new VulkanInstance(flags.width, flags.height, flags.max_frames_in_flight, flags.RTXEnable));
			m_Camera = Camera(45.0f, flags.width, flags.height, 0.1f, 10.0f, glm::vec3(0, 0, 0), m_VulkanInstance->getGLFWwindow());
		}

		void render(Renderable* renderableStart, uint32_t count)
		{
			static auto startTime = std::chrono::high_resolution_clock::now();

			auto currentTime = std::chrono::high_resolution_clock::now();
			float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

			glfwPollEvents();

			m_VulkanInstance->render(time, renderableStart, count);
			frameCounter++;
			if (time - counter >= 1)
			{
				std::cout << "FPS: " << frameCounter << std::endl;
				counter++;
				frameCounter = 0;
			}
		}

		bool shouldClose()
		{
			return m_VulkanInstance->shouldClose();
		}

		std::shared_ptr<VulkanInstance> getVulkan()
		{
			return m_VulkanInstance;
		}

		WindowFlags getFlags()
		{
			return m_Flags;
		}

	private:
		Camera m_Camera;
		std::shared_ptr<VulkanInstance> m_VulkanInstance;
		WindowFlags m_Flags;

		int counter = 0;
		int frameCounter = 0;
	};
}