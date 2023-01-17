#pragma once
#include "OS-Dependant/Vulkan/VulkanInstance.h"
#include "Clever/Camera/Camera.h"
#include "Clever/WorldManager/Components/Component/Renderable.h"
#include "OS-Dependant/ImGui/ImGuiManager.h"
#include "Clever/Developer/DockManager.h"
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
			bool DeveloperMode;
			bool RTXEnable;
		};

	public:
		WindowManager()
		{

		}

		static void testWindow()
		{
			DevTools::newDock("Test");
			DevTools::coloredText({0.5, 0.2, 0.7}, "Test-Test");
			DevTools::endDock();
		}

		void WindowInit(WindowFlags flags = {})
		{
			m_Flags = flags;
			m_VulkanInstance.reset(new VulkanInstance(flags.width, flags.height, flags.max_frames_in_flight, flags.RTXEnable));
			m_VulkanInstance->m_Camera->init();
			if (m_Flags.DeveloperMode)
			{
				m_ImGuiManager = ImGuiManager();
				m_ImGuiManager.bind(m_VulkanInstance, m_CurrentFrame);
			}
			
			DevTools::addDockFunction(testWindow, {this});
		}

		void render(Renderable* renderableStart, uint32_t count, float deltaTime)
		{	
			uint32_t imageIndex = -1;
			VkCommandBuffer ImGuiCommandBuffer = VK_NULL_HANDLE;
			if (m_Flags.DeveloperMode)
			{
				m_ImGuiManager.newFrame();

				DevTools::BuildCustomUI();

				imageIndex = m_ImGuiManager.render(m_VulkanInstance, m_CurrentFrame);
				if (imageIndex != -1)
					ImGuiCommandBuffer = m_ImGuiManager.getCommandBuffer(m_CurrentFrame);
				else
					m_ImGuiManager.endFrame();
			}
			bool recreate = m_VulkanInstance->render(deltaTime, renderableStart, ImGuiCommandBuffer, count, m_CurrentFrame, imageIndex);
			if (recreate)
			{
				m_ImGuiManager.recreateFrameBuffer(m_VulkanInstance);
			}


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

			if (m_Flags.DeveloperMode)
			{
				ImGui_ImplVulkan_Shutdown();
				ImGui_ImplGlfw_Shutdown();
				ImGui::DestroyContext();
				m_ImGuiManager.cleanup(m_VulkanInstance);
			}
			
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

		Camera getCamera()
		{
			return *m_VulkanInstance->m_Camera;
		}

	private:
		std::shared_ptr<VulkanInstance> m_VulkanInstance;
		ImGuiManager m_ImGuiManager;
		WindowFlags m_Flags;

		uint32_t m_CurrentFrame = 0;
	};
}