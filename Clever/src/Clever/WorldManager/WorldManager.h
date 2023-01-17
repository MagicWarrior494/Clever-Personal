#pragma once
#include "Components/ComponentManager.h"
#include "OS-Dependant/Vulkan/VulkanInstance.h"
#include "Object/ObjectManager.h"
#include "Clever/Developer/DevTools.h"
#include "Clever/EventSystem/EventManager.h"
#include <iostream>
#include <string>

namespace World
{

	struct WorldFlags
	{
		std::string WorldFileLocation;
	};

	class WorldManager
	{
	public:

		void addRay()
		{
			int count = ray.getInstanceCount();
			ray.setInstanceCount(count + 1);
			//window.getVulkan()->m_Camera.GetPosition() + 
			ray.setLocation({ camera->GetPosition() + (camera->GetRotation() * 3.0f)}, count);
			componentManager.changeEntityComponent(1, ray);
		}

		Renderable getRay()
		{
			return ray;
		}

		static void worldUI(std::vector<void*> classInstances)
		{
			WorldManager* world = (WorldManager*)classInstances.at(0);
			DevTools::newDock("Entities");
			DevTools::coloredText(glm::vec3(0.25, 0.76, 0.50), "This is PRetty Cool");
			if (DevTools::button("AddRay"))
			{  
				world->addRay();
			}
			DevTools::endDock();
		}

		static void worldUI2 (std::vector<void*> classInstances)
		{
			DevTools::newDock("fdsafads");
			DevTools::coloredText(glm::vec3(0.25, 0.76, 0.50), "Seonds Window test");
			DevTools::endDock();
		}

		WorldManager()
		{
			
		}
		~WorldManager()
		{

		}
		void worldInit(std::shared_ptr<VulkanInstance> vulkanInstance, WorldFlags flags = {})
		{

			//Registering INGUI window dock function
			DevTools::addDockFunction(worldUI, { this });
			DevTools::addDockFunction(worldUI2, { this });

			camera = vulkanInstance->m_Camera;

			//Registering All Components before any entities are created
			{

				componentManager.RegisterComponent<Renderable>();

			}
			ray = { vulkanInstance->m_Device, vulkanInstance->m_PhysicalDevice, vulkanInstance->m_RenderPass, vulkanInstance->m_CommandPool, vulkanInstance->m_GraphicsQueue, vulkanInstance->m_UniformBuffers, vulkanInstance->m_max_frames_in_flight, true };
			loadedObject = { vulkanInstance->m_Device, vulkanInstance->m_PhysicalDevice, vulkanInstance->m_RenderPass, vulkanInstance->m_CommandPool, vulkanInstance->m_GraphicsQueue, vulkanInstance->m_UniformBuffers, vulkanInstance->m_max_frames_in_flight, false };

			{
				std::pair<std::vector<Vertex>, std::vector<uint16_t>> teapotModel = loadModel("D:/Clever-Personal/Clever/Clever/Resource/Models/Teapot.obj");

				std::pair<std::vector<Vertex>, std::vector<uint16_t>> rayModel = {vertices, indices };

				componentManager.AddEntity();
				componentManager.AddEntity();

				loadedObject.setComponentData(teapotModel);
				loadedObject.setLocation({ 0, 0, 0 });

				ray.setComponentData(rayModel);
				ray.setLocation({ 2,2,2 });

				componentManager.changeEntityComponent(0, loadedObject);
				componentManager.changeEntityComponent(1, ray);
				//componentManager.changeEntityComponent(1, loadedObject);
				//Creating a Renderable Object with needed data for a Cube
			}

			//addRay();
		}

		void update()
		{
			if (Event::EventManager::isKeyPressed(KEY_R))
			{
				addRay();
			}
		}

		Renderable* getRenderables()
		{
			return componentManager.getComponentArray<Renderable>();
		}

		int getRenderablesSize()
		{
			return componentManager.getComponentArraySize<Renderable>();
		}

		void cleanup()
		{
			
		}

	private:
		ComponentManager componentManager{};

		std::shared_ptr<Camera> camera;

		Renderable ray;
		Renderable loadedObject;

		std::vector<Vertex> vertices = {
		{{0.0, -1.0, 0.0}, {0.2f, 0.1f, 0.5f}},
		{{0.0, 1.0, 0.0}, {0.1f, 0.7f, 0.5f}},
		};

		std::vector<uint16_t> indices = {
			0, 1, 0
		};
	};

}