#pragma once
#include "OS-Dependant/Vulkan/VulkanInstance.h"
#include "Clever/Camera/Camera.h"
#include "Clever/WorldManager/Components/Component/Renderable.h"
#include "OS-Dependant/ImGui/ImGuiManager.h"
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

	public:
		WindowManager()
		{

		}

		void WindowInit(WindowFlags flags = {})
		{
			m_Flags = flags;
			m_VulkanInstance.reset(new VulkanInstance(flags.width, flags.height, flags.max_frames_in_flight, flags.RTXEnable));
			m_ImGuiManager = ImGuiManager();
			m_ImGuiManager.bind(m_VulkanInstance, m_CurrentFrame);
			m_Camera = Camera(90.0f, flags.width, flags.height, 0.1f, 10.0f, glm::vec3(0, 0, 0), m_VulkanInstance->getGLFWwindow());
		}

		void render(Renderable* renderableStart, uint32_t count)
		{
			static auto startTime = std::chrono::high_resolution_clock::now();

			auto currentTime = std::chrono::high_resolution_clock::now();
			float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
			float timeDelta = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();

			glfwPollEvents();

			m_ImGuiManager.newFrame();
			m_ImGuiManager.showDemoWindow();

			m_VulkanInstance->render(timeDelta, renderableStart, count, m_CurrentFrame);
			frameCounter++;
			if (time - counter >= 1)
			{
				std::cout << "FPS: " << frameCounter << std::endl;
				counter++;
				frameCounter = 0;
			}
			lastTime = std::chrono::high_resolution_clock::now();

			m_CurrentFrame = (m_CurrentFrame + 1) % m_VulkanInstance->m_max_frames_in_flight;
		}

		void cleanup(Renderable* renderableStart, uint32_t count)
		{
			vkWaitForFences(m_VulkanInstance->m_Device, 1, &m_VulkanInstance->m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);
			vkDeviceWaitIdle(m_VulkanInstance->m_Device);

			Renderable* renderables = renderableStart;

			for (int i = 0; i < count; i++)
			{
				renderables->destory();
				renderables++;
			}

			ImGui_ImplVulkan_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();

			m_ImGuiManager.cleanup(m_VulkanInstance);
			m_VulkanInstance->cleanup();
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
		ImGuiManager m_ImGuiManager;
		WindowFlags m_Flags;

		uint32_t m_CurrentFrame = 0;

		int counter = 0;
		int frameCounter = 0;
		std::chrono::time_point<std::chrono::steady_clock> lastTime;
	};
}